#ifndef _TIMER_QUEUE_H_TLKIT__
#define _TIMER_QUEUE_H_TLKIT__

#include "../../utils/NonCopyable.h"
#include "../callbacks.h"
#include "Timer.h"
#include <queue>
#include <memory>
#include <atomic>
#include <unordered_set>

namespace trantor {
    // class Timer;
    class EventLoop;
    class Channel;
    using TimerPtr = std::shared_ptr<Timer>;

    struct TimerPtrComparer {
        bool operator()(const TimerPtr &x, const TimerPtr &y) const { return *x > *y; }
    };

    class TimerQueue : NonCopyable {
    public:
        explicit TimerQueue(EventLoop *loop);
        ~TimerQueue();
        /**
         * @return id timerid
         */
        TimerId addTimer(const TimerCallback &cb, const TimePoint &when, const TimeInterval &interval);
        TimerId addTimer(TimerCallback &&cb, const TimePoint &when, const TimeInterval &inverval);

        void addTimerInLoop(const TimerPtr &timer);
        void invalidateTimer(TimerId id);
        #ifdef __linux__
        void reset();
        #else 
            int64_t getTimeout() const;
            void processTimers();
        #endif

    protected:
        EventLoop *loop_;
    #ifdef __linux__
        int timerfd_;
        std::shared_ptr<Channel> timerfdChannelPtr_;
        void handleRead();
    #endif
        std::priority_queue<TimerPtr, std::vector<TimerPtr>,TimerPtrComparer> timers_;
        bool callingExpiredTimers_;
        bool insert(const TimerPtr &timerPtr);
        std::vector<TimerPtr> getExpired();
        void reset(const std::vector<TimerPtr> &expired, const TimePoint &now);
        std::vector<TimerPtr> getExpired(const TimePoint &now);

    private:
        std::unordered_set<uint64_t> timerIdSet_;
    };
}  // namespace trantor

#endif 