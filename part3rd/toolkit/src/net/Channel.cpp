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
    const int Channel::kWriteEvent = ;
}  // namespace trantor
