#include "Channel.h"
#include "EventLoop.h"
#ifdef _WIN32
#include "Wepoll.h"
#define POLLIN EPOLLIN
#define POLLPRI EPOLLPRI
#define POLLOUT EPOLLOUT
#define POLLHUP EPOLLHUP
#define POLLNVAL 0
#define POLLERR EPOLLERR
#else
#include <poll.h>
#endif
#include <iostream>

namespace trantor {
    const int Channel::kNoneEvent = 0;
    const int Channel::kReadEvent = POLLIN | POLLPRI;
    const int Channel::kWriteEvent = POLLOUT;

    Channel::Channel(EventLoop *loop, int fd) 
        : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) 
        
    {}

    void Channel::remove() {
        assert(events_ == kNoneEvent);
        addedToLoop_ = false;
        loop_->removeChannel(this);
    }

}  // namespace trantor
