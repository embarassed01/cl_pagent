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
#include <stdlib.h>

#include "wepoll.h"

static void create_and_add_socket(HANDLE ephnd, int af, int type, int protocol) {
    SOCKET sock;
    u_long one = 1;
    int r;
    struct epoll_event ev;

    sock = socket(af, type, protocol);
    check(sock != INVALID_SOCKET);

    r = ioctlsocket(sock, (long)FIONBIO, &one);
    check(r == 0);

    ev.events = EPOLLOUT | EPOLLONESHOT;
    ev.data.sock = sock;
    r = epoll_ctl(ephnd, EPOLL_CTL_ADD, sock, &ev);
    check(r == 0);
}

int main(void) {
    printf("test mixed socket types\n");

    HANDLE ephnd;
    int i, r;
    struct epoll_event ev;

    ephnd = epoll_create1(0);
    check(ephnd != NULL);

    create_and_add_socket(ephnd, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    create_and_add_socket(ephnd, AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    for (i = 0; i < 2; i++) {
        r = epoll_wait(ephnd, &ev, 1, -1);
        check(r == 1);

        r = epoll_ctl(ephnd, EPOLL_CTL_DEL, ev.data.sock, NULL);
        check(r == 0);

        r = closesocket(ev.data.sock);
        check(r == 0);
    }

    r = epoll_close(ephnd);
    check(r == 0);

    return 0;
}