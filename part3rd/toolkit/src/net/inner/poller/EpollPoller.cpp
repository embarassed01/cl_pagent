#include "utils/Logger.h"
#include "net/Channel.h"
#include "EpollPoller.h"
#ifdef __linux__
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <iostream>
#elif defined _WIN32
#include "Wepoll.h"
#include <assert.h>
#include <iostream>
#include <winsock2.h>
#include <fcntl.h>
#define EPOLL_CLOEXEC  _O_NOINHERIT
#endif

