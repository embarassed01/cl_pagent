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

// ###############################################################
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "wepoll.h"

#define LISTEN_PORT 12345
#define NUM_PINGERS 10000
#define RUN_TIME 5000

static const char PING[] = "PING";

int main(void) {
    printf("test udp pings\n");

    HANDLE ephnd;
    int r;
    u_long one = 1;
    struct sockaddr_in address;
    DWORD ticks_start, ticks_last;
    uint64_t pings = 0, pings_sent = 0;
    int i;
    SOCKET srv;
    struct epoll_event ev;

    ephnd = epoll_create1(0);
    check(ephnd != NULL);

    srv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    r = ioctlsocket(srv, (long)FIONBIO, &one);
    check(r == 0);

    address.sin_family = AF_INET;
    address.sin_port = htons(LISTEN_PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // #define INADDR_LOOPBACK 0x7f000001
    r = bind(srv, (struct sockaddr*)&address, sizeof address);
    check(r == 0);

    ev.events = EPOLLIN | EPOLLERR;
    ev.data.sock = srv;
    r = epoll_ctl(ephnd, EPOLL_CTL_ADD, srv, &ev);
    check(r == 0);

    for (i = 0; i < NUM_PINGERS; i++) {
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        r = ioctlsocket(sock, (long)FIONBIO, &one);
        check(r == 0);
        r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof one);
        check(r == 0);
        r = connect(sock, (struct sockaddr*)&address, sizeof address);
        /* Unlike Unix, Windows set the error to EWOULDBLOCK when the connection is being established in the background. */
        check(r == 0 || WSAGetLastError() == WSAEWOULDBLOCK);
        ev.events = EPOLLOUT | EPOLLERR | EPOLLONESHOT;
        ev.data.sock = sock;
        r = epoll_ctl(ephnd, EPOLL_CTL_ADD, sock, &ev);
        check(r == 0);
    }

    ticks_start = GetTickCount(); 
    ticks_last = ticks_start;

    for (;;) {
        int count;
        struct epoll_event events[16];
        DWORD ticks;
        ticks = GetTickCount();
        if (ticks >= ticks_last + 1000) {
            printf("%I64d pings (%0.0f per sec), %I64d sent (%0.0f per sec)\n", pings, (double)pings/(ticks - ticks_start) * 1000.0, pings_sent, (double)pings_sent/(ticks-ticks_start)*1000.0);
            ticks_last = ticks;
            if (ticks - ticks_start > RUN_TIME) break;
        }
        count = epoll_wait(ephnd, events, 15, 1000);
        check(count >= 0);
        
        for (i = 0; i < count; i++) {
            uint32_t revents = events[i].events;

            if (revents & EPOLLERR) {
                SOCKET sock = events[i].data.sock;
                int err = -1;
                int err_len = sizeof err;
                r = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &err_len);
                check(r == 0);
                fprintf(stderr, "Socket error: %d\n", err);
                r = epoll_ctl(ephnd, EPOLL_CTL_DEL, sock, NULL);
                check(r == 0);
                continue;
            }

            if (revents & EPOLLIN) {
                SOCKET sock = events[i].data.sock;
                char buf[1024];
                WSABUF wsa_buf;
                DWORD flags, bytes;
                wsa_buf.buf = buf;
                wsa_buf.len = sizeof buf;
                flags = 0;
                for (;;) {
                    r = WSARecv(sock, &wsa_buf, 1, &bytes, &flags, NULL, NULL);
                    if (r == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) break;
                    check(r >= 0);
                    check(bytes == sizeof PING);
                    pings++;
                }
                continue;
            }

            if (revents & EPOLLOUT) {
                SOCKET sock = events[i].data.sock;
                WSABUF wsa_buf;
                DWORD bytes;
                wsa_buf.buf = (char*)PING;
                wsa_buf.len = sizeof PING;
                r = WSASend(sock, &wsa_buf, 1, &bytes, 0, NULL, NULL);
                check(r >= 0);
                check(bytes == sizeof PING);
                pings_sent++;
                uint32_t rev = EPOLLOUT | EPOLLONESHOT;
                struct epoll_event e;
                e.data.sock = sock;
                e.events = rev;
                if (epoll_ctl(ephnd, EPOLL_CTL_MOD, sock, &e) < 0) abort();
                continue;
            }
            check(0);
        }
    }

    r = epoll_close(ephnd);
    check(r == 0);
    closesocket(srv);
    return 0;
}