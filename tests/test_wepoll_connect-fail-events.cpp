#ifndef TEST_UTIL_H_
#define TEST_UTIL_H_
#include <assert.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wbad-function-cast"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wformat-non-iso"
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#ifdef _MSC_VER
#define no_return __declspec(noreturn)
#else /* GCC/Clang */
#define no_return __attribute__((noreturn))
#endif

#ifdef _MSC_VER
#define no_inline __declspec(noinline)
#else /* GCC/Clang */
#define no_inline __attribute__((noinline))
#endif

void no_inline no_return check_fail(const char* message);

#define test_util__to_string_helper(v) #v
#define test_util__to_string(v) test_util__to_string_helper(v)

#define check(expression)          \
  (void) ((!!(expression)) ||          \
          (check_fail("\n"                                             \
                      "Check failed:\n"                                \
                      "  test: " #expression "\n"                      \
                      "  file: " __FILE__ "\n"                         \
                      "  line: " test_util__to_string(__LINE__) "\n"), \
           0))

/* Polyfill `static_assert` for some versions of clang and gcc. */
#if (defined(__clang__) || defined(__GNUC__)) && !defined(static_assert)
#define static_assert(condition, message) \
  typedef __attribute__(                  \
      (__unused__)) int __static_assert_##__LINE__[(condition) ? 1 : -1]
#endif
#endif /* TEST_UTIL_H_ */
#include <stdio.h>
#include <stdlib.h>
void check_fail(const char* message) {
  puts(message);
  abort();
}

#ifndef WEPOLL_WIN_H_
#define WEPOLL_WIN_H_
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined(_MSC_VER)
#pragma warning(push, 1)
#endif

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600

/* clang-format off */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
/* clang-format on */

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
#endif /* WEPOLL_WIN_H_ */

#ifndef WEPOLL_UTIL_H_
#define WEPOLL_UTIL_H_
#include <stddef.h>
#include <stdint.h>

#define array_count(a) (sizeof(a) / (sizeof((a)[0])))

/* clang-format off */
#define container_of(ptr, type, member) \
  ((type*) ((uintptr_t) (ptr) - offsetof(type, member)))
/* clang-format on */

#define unused_var(v) ((void) (v))

/* Polyfill `inline` for older versions of msvc (up to Visual Studio 2013) */
#if defined(_MSC_VER) && _MSC_VER < 1900
#define inline __inline
#endif
#endif /* WEPOLL_UTIL_H_ */

// ###############################################################
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "wepoll.h"

#define FAIL_PORT 1

int main() {
    printf("test connect fail events\n");

    unsigned long one = 1;
    HANDLE ephnd;
    SOCKET sock;
    struct epoll_event ev;
    struct sockaddr_in addr;
    int err, len;
    int r;
    /* Create epoll port. */
    ephnd = epoll_create1(0);
    check(ephnd != NULL);
    /* Create a stream socket, switch it to non-blocking mode. */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    check(sock != INVALID_SOCKET);
    r = ioctlsocket(sock, (long)FIONBIO, &one);
    check(r == 0);

    /* Associate the socket with the epoll port; set it up such that we'll pick up any valid event. */
    ev.events = EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDNORM | EPOLLRDBAND | EPOLLWRNORM | EPOLLWRBAND | EPOLLMSG | EPOLLRDHUP;
    ev.data.sock = sock;
    r = epoll_ctl(ephnd, EPOLL_CTL_ADD, sock, &ev);
    check(r == 0);

    /* Attempt to connect to an address that we won't be able to connect to. */
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(FAIL_PORT);
    r = connect(sock, (struct sockaddr*)&addr, sizeof addr);
    check(r == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK);

    /* Wait for the socket to report an event, and verify that the events reported match exactly what the linux kernel would report in case of a connect() failure. */
    r = epoll_wait(ephnd, &ev, 1, -1);
    check(r == 1);
    check(ev.data.sock == sock);
    check(ev.events == (EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDNORM | EPOLLWRNORM | EPOLLRDHUP));

    /* Retrieve the error associated with the socket, and verify that connect() failed because the server refused the connection. */
    len = sizeof err;
    r = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    check(r == 0);
    check(err == WSAECONNREFUSED);

    /* Clean up. */
    r = closesocket(sock);
    check(r == 0);
    r = epoll_close(ephnd);
    check(r == 0);

    return 0;
}