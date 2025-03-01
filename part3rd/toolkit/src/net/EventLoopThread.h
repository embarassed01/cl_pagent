#ifndef _EVENTLOOPTHREAD_H_TLKIT__
#define _EVENTLOOPTHREAD_H_TLKIT__

#include "EventLoop.h"
#include "../utils/NonCopyable.h"
#include "../exports.h"
#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>
#include <future>

namespace trantor {
    /**
     * @brief This class represents an event loop thread
     */
    class TRANTOR_EXPORT EventLoopThread : NonCopyable {
        public:
        explicit EventLoopThread(const std::string &threadName = "EventLoopThread");
        ~EventLoopThread();

        /**
         * @brief Wait for the event loop to exit.
         * @note This method blocks the current thread until the event loop exits.
         */
        void wait();

        /**
         * @brief Get the pointer of the event loop of the thread.
         * @return EventLoop*
         */
        EventLoop * getLoop() const {
            return loop_.get();
        }

        /**
         * @brief Run the event loop of the thread. This method doesn't block the current thread.
         */
        void run();

    private:
        // With C++20, use std::atomic<std::shared_ptr<EventLoop>>
        std::shared_ptr<EventLoop> loop_;
        std::mutex loopMutex_;
        std::string loopThreadName_;
        void loopFuncs();
        std::promise<std::shared_ptr<EventLoop>> promiseForLoopPointer_;
        std::promise<int> promiseForRun_;
        std::promise<int> promiseForLoop_;
        std::once_flag once_;
        std::thread thread_;
    };
    
}  // namespace trantor

#endif 