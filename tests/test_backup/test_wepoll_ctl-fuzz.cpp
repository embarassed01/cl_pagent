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
extern int init();

#define NUM_SOCKETS 10000
#define RUN_TIME 5000
#define PRINT_INTERVAL 500

static SOCKET create_and_add_socket(HANDLE epfd) {
    SOCKET sock;
    unsigned long one = 1;
    int r;
    struct epoll_event ev;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    check(sock > 0);

    r = ioctlsocket(sock, (long)FIONBIO, &one);
    check(r == 0);

    ev.events = 0;
    ev.data.u64 = 42;
    r = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
    check(r == 0);

    return sock;
}

int main(void) {
    printf("test ctl fuzz\n");

    uint64_t total_events = 0;
    uint64_t start_time, last_print_time, now, total_time;
    SOCKET sockets[NUM_SOCKETS];
    HANDLE epfd;
    size_t i;
    int r;

    r = init();
    check(r == 0);

    epfd = epoll_create1(0);
    check(epfd != NULL);
    
    for (i = 0; i < NUM_SOCKETS; i++) {
        sockets[i] = create_and_add_socket(epfd);
    }

    start_time = GetTickCount64();
    last_print_time = 0;

    do {
        struct epoll_event ev_out[64];
        uint64_t count;

        for (i = 0; i < NUM_SOCKETS; i++) {
            SOCKET sock = sockets[i];
            struct epoll_event ev_in;
            ev_in.events = (rand() & 0xff) | EPOLLONESHOT;
            ev_in.data.u64 = 42;
            r = epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &ev_in);
            check(r == 0);
        }

        count = 0;
        do {
            r = epoll_wait(epfd, ev_out, array_count(ev_out), count > 0 ? 0: -1);
            check(r >= 0);
            count += (uint64_t)r;
        } while (r > 0);

        total_events += count;

        now = GetTickCount64();
        total_time = now - start_time;

        if (now - last_print_time > PRINT_INTERVAL) {
            last_print_time = now;
            printf("%f events (%f events/sec)\n", (double)total_events, (double)total_events / total_time * 1000);
        }
    } while (total_time < RUN_TIME);

    epoll_close(epfd);

    return 0;
}