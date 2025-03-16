#pragma once

#include "trantor/net/EventLoop.h"
#include "trantor/utils/Logger.h"
#include <atomic>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <future>
#include <assert.h>

#define WHEELS_NUM 4
#define BUCKET_NUM_PER_WHEEL 200
#define TICK_INTERVAL 1.0

namespace drogon
{
/**
 * @brief A utility class for CacheMap
 */
class CallbackEntry
{
public:
    CallbackEntry(std::function<void()> cb) : cb_(std::move(cb))
    {
    }

    ~CallbackEntry()
    {
        cb_();
    }

private:
    std::function<void()> cb_;
};

using CallbackEntryPtr = std::shared_ptr<CallbackEntry>;
using WeakCallbackEntryPtr = std::weak_ptr<CallbackEntry>;

using CallbackBucket = std::unordered_set<CallbackEntryPtr>;
using CallbackBucketQueue = std::deque<CallbackBucket>;

/**
 * @brief Cache Map
 * 
 * @tparam T1 The keyword type. 
 * @tparam T2 The value type. 
 * @note Four wheels with 200 buckets per wheel means the cache map can work with a timeout up to 200^4 seconds (about 50 years).
 */
template <typename T1, typename T2>
class CacheMap
{
public:
    /// constructor
    /**
     * @param loop eventloop pointer
     * @param tickInterval second
     * @param wheelsNum number of wheels
     * @param bucketsNumPerWheel buckets number per wheel
     * @param fnOnInsert function to execute on insertion
     * @param fnOnErase function to execute on erase
     * @details The max delay of the CacheMap is about `tickInterval * (bucketsNumPerWheel ^ wheelsNum)` seconds
    */
    CacheMap(trantor::EventLoop *loop,
            float tickInterval = TICK_INTERVAL,
            size_t wheelsNum = WHEELS_NUM,
            size_t bucketsNumPerWheel = BUCKET_NUM_PER_WHEEL,
            std::function<void(const T1 &)> fnOnInsert = nullptr,
            std::function<void(const T1 &)> fnOnErase = nullptr)
        : loop_(loop),
          tickInterval_(tickInterval),
          wheelsNumber_(wheelsNum),
          bucketsNumPerWheel_(bucketsNumPerWheel),
          ctrlBlockPtr_(std::make_shared<ControlBlock>()),
          fnOnInsert_(fnOnInsert),
          fnOnErase_(fnOnErase)
    {
        wheels_.resize(wheelsNumber_);
        for (size_t i = 0; i < wheelsNumber_; ++i)
        {
            wheels_[i].resize(bucketsNumPerWheel_);
        }
        if (tickInterval_ > 0 && wheelsNumber_ > 0 && bucketsNumPerWheel_ > 0)
        {
            timerId_ = loop_->runEvery(
                tickInterval_, [this, ctrlBlockPtr = ctrlBlockPtr_]() {
                    std::lock_guard<std::mutex> lock(ctrlBlockPtr->mtx);
                    if (ctrlBlockPtr->destructed)
                        return;
                    
                    size_t t = ++ticksCounter_;
                    size_t pow = 1;
                    for (size_t i = 0; i < wheelsNumber_; ++i)
                    {
                        if ((t % pow) == 0)
                        {
                            CallbackBucket tmp;
                            {
                                std::lock_guard<std::mutex> lock(bucketMutex_);
                                // use `tmp` val to make this critical area as short as possible.
                                wheels_[i].front().swap(tmp);
                                wheels_[i].pop_front();
                                wheels_[i].push_back(CallbackBucket());
                            }
                        }
                        pow = pow * bucketsNumPerWheel_;
                    }
                });
            loop_->runOnQuit([ctrlBlockPtr = ctrlBlockPtr_]{
                std::lock_guard<std::mutex> lock(ctrlBlockPtr->mtx);
                ctrlBlockPtr->loopEnded = true;
            });
        }
        else 
        {
            noWheels_ = true;
        }
    };

    ~CacheMap()
    {
        std::lock_guard<std::mutex> lock(ctrlBlockPtr_->mtx);
        ctrlBlockPtr_->destructed = true;
        map_.clear();
        if (!ctrlBlockPtr_->loopEnded)
        {
            loop_->invalidateTimer(timerId_);
        }
        for (auto iter = wheels_.rbegin(); iter != wheels_.rend(); ++iter)
        {
            iter->clear();
        }
        LOG_TRACE << "CacheMap destruct!";
    }

    struct MapValue
    {
        MapValue(const T2 &value,
                size_t timeout, 
                std::function<void()> &&callback)
            : value_(value),
              timeout_(timeout),
              timeoutCallback_(std::move(callback))
        {
        }

        MapValue(T2 &&value, size_t timeout, std::function<void()> &&callback)
            : value_(std::move(value)),
            timeout_(timeout),
            timeoutCallback_(std::move(callback))
        {
        }

        MapValue(T2 &&value, size_t timeout)
            : value_(std::move(value)), 
            timeout_(timeout)
        {
        }

        MapValue(const T2 &value, size_t timeout)
            : value_(value),
            timeout_(timeout)
        {
        }

        MapValue(T2 &&value)
            : value_(std::move(value))
        {
        }

        MapValue(const T2 &value)
            : value_(value)
        {
        }

        MapValue() = default;
        T2 value_;
        size_t timeout_{0};
        std::function<void()> timeoutCallback_;
        WeakCallbackEntryPtr weakEntryPtr_;
    };

    /**
     * @brief Insert a key-value pair into the cache.
     * @param key The key 
     * @param value The value 
     * @param timeout The timeout in seconds, if timeout > 0, the value will be erased within the 'timeout' seconds after the last access. If the timeout is zero, the value exists until being removed explicitly. 
     * @param timeoutCallback is called when the timeout expires. 
    */
    void insert(const T1 &key, 
                T2 &&value, 
                size_t timeout = 0,
                std::function<void()> timeoutCallback = std::function<void()>())
    {
        if (timeout > 0)
        {
            MapValue v{std::move(value), timeout, std::move(timeoutCallback)};
            std::lock_guard<std::mutex> lock(mtx_);
            map_.insert(std::make_pair(key, std::move(v)));
            eraseAfter(timeout, key);
        }
        else  
        {
            MapValue v{std::move(value)};
            std::lock_guard<std::mutex> lock(mtx_);
            map_.insert(std::make_pair(key, std::move(v)));
        }
        if (fnOnInsert_)
            fnOnInsert_(key);
    }

private:
    /**
     * @brief ControlBlock in a internal structure that deals with synchronizing CacheMap destructing, event loop destructing and updating the Cachemap. 
     * It is possible that the EventLoop destructed before the CacheMap (ex: both Cachemap and the EventLoop being globals, the order of destruction is not defined), thus we shouldn't invalidate the time. Or CacheMap destructed before the event loop but the timer is still active. Thus we should avoid updating the CacheMap.
     */
    struct ControlBlock
    {
        ControlBlock() : destructed(false), loopEnded(false)
        {
        }

        bool destructed;
        bool loopEnded;
        std::mutex mtx;
    };

    std::unordered_map<T1, MapValue> map_;
    std::vector<CallbackBucketQueue> wheels_;
    std::atomic<size_t> ticksCounter_{0};
    std::mutex mtx_;
    std::mutex bucketMutex_;
    trantor::TimerId timerId_;
    trantor::EventLoop *loop_;

    float tickInterval_;
    size_t wheelsNumber_;
    size_t bucketsNumPerWheel_;
    std::shared_ptr<ControlBlock> ctrlBlockPtr_;
    std::function<void(const T1 &)> fnOnInsert_;
    std::function<void(const T1 &)> fnOnErase_;

    bool noWheels_{false};

    void insertEntry(size_t delay, CallbackEntryPtr entryPtr)
    {
        // protected by bucketMutex;
        if (delay <= 0)
            return;
        delay = static_cast<size_t>(delay / tickInterval_ + 1);
        size_t t = ticksCounter_;
        for (size_t i = 0; i < wheelsNumber_; ++i)
        {
            if (delay <= bucketsNumPerWheel_)
            {
                wheels_[i][delay - 1].insert(entryPtr);
                break;
            }
            if (i < (wheelsNumber_ - 1))
            {
                entryPtr = std::make_shared<CallbackEntry>(
                    [this, delay, i, t, entryPtr]() {
                        if (delay > 0)
                        {
                            std::lock_guard<std::mutex> lock(bucketMutex_);
                            wheels_[i][(delay + (t % bucketsNumPerWheel_) - 1) % bucketsNumPerWheel_].insert(entryPtr);
                        }
                    });
            }
            else 
            {
                // delay is too long to put entry at valid position in wheels;
                wheels_[i][bucketsNumPerWheel_ - 1].insert(entryPtr);
            }
            delay = (delay + (t % bucketsNumPerWheel_) - 1) / bucketsNumPerWheel_;
            t = t / bucketsNumPerWheel_;
        }
    }

    void eraseAfter(size_t delay, const T1 &key)
    {
        if (noWheels_)
            return;
        assert(map_.find(key) != map_.end());

        CallbackEntryPtr entryPtr;

        if (map_.find(key) != map_.end())
        {
            entryPtr = map_[key].weakEntryPtr_.lock();
        }

        if (entryPtr)
        {
            std::lock_guard<std::mutex> lock(bucketMutex_);
            insertEntry(delay, entryPtr);
        }
        else 
        {
            std::function<void()> cb = [this, key]() {
                bool erased{false};
                std::function<void()> timeoutCallback;
                {
                    std::lock_guard<std::mutex> lock(mtx_);
                    auto iter = map_.find(key);
                    if (iter != map_.end())
                    {
                        auto &value = iter->second;
                        auto entryPtr = value.weakEntryPtr_.lock();
                        // entryPtr is used to avoid race conditions
                        if (value.timeout_ > 0 && !entryPtr)
                        {
                            erased = true;
                            timeoutCallback = std::move(value.timeoutCallback_);
                            map_.erase(key);
                        }
                    }
                }
                if (erased && fnOnErase_)
                    fnOnErase_(key);
                if (erased && timeoutCallback)
                    timeoutCallback();
            };
            entryPtr = std::make_shared<CallbackEntry>(std::move(cb));
            map_[key].weakEntryPtr_ = entryPtr;
            {
                std::lock_guard<std::mutex> lock(bucketMutex_);
                insertEntry(delay, entryPtr);
            }
        }
    }
};
}  // namespace drogon