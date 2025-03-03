/**
 * verifies that sockets are automatically dropped from an epoll set when they are closed.
 * 
 * On Linux, a socket would be dropped from the epoll set immediately立即 after the call to close().
 * However, on Windows, this isn't possible - wepoll may not detect that a socket has been closed until a call to epoll_wait() is made.
 */
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
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "wepoll.h"

static SOCKET create_and_add_socket(HANDLE ephnd, uint32_t events) {
  SOCKET sock;
  struct epoll_event ev;
  int r;

  /* Create UDP socket. */
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  check(sock != INVALID_SOCKET);

  /* Associate with epoll port. */
  ev.events = events;
  ev.data.sock = sock;
  r = epoll_ctl(ephnd, EPOLL_CTL_ADD, sock, &ev);
  check(r == 0);

  return sock;
}

static void check_dropped(HANDLE ephnd, SOCKET sock) {
  struct epoll_event ev;
  int r;

  ev.events = EPOLLERR | EPOLLHUP;
  ev.data.u64 = 0;

  /* Check that EPOLL_CTL_MOD fails. */
  r = epoll_ctl(ephnd, EPOLL_CTL_MOD, sock, &ev);
  check(r < 0);
  check(errno == EBADF);

  /* Check that EPOLL_CTL_DEL fails. */
  r = epoll_ctl(ephnd, EPOLL_CTL_DEL, sock, &ev);
  check(r < 0);
  check(errno == EBADF);
}

int main() {
  printf("test auto drop on close\n");

  HANDLE ephnd;
  SOCKET sock1, sock2, sock3, sock4, sock5;
  struct epoll_event evs[8];
  int r;

  ephnd = epoll_create1(0);
  check(ephnd != NULL);

  sock1 = create_and_add_socket(ephnd, EPOLLIN);
  sock2 = create_and_add_socket(ephnd, EPOLLIN);
  sock3 = create_and_add_socket(ephnd, EPOLLOUT);
  sock4 = create_and_add_socket(ephnd, EPOLLOUT | EPOLLONESHOT);
  sock5 = create_and_add_socket(ephnd, 0);

  r = closesocket(sock1);
  check(r == 0);

  r = epoll_wait(ephnd, evs, array_count(evs), -1);
  check(r == 2); /* sock3 and sock4 should report events. */

  check_dropped(ephnd, sock1);

  r = closesocket(sock2);
  check(r == 0);
  r = closesocket(sock3);
  check(r == 0);

  r = epoll_wait(ephnd, evs, array_count(evs), 0);
  check(r == 0); /* No events should be reported. */

  check_dropped(ephnd, sock2);
  check_dropped(ephnd, sock3);

  r = closesocket(sock4);
  check(r == 0);
  r = closesocket(sock5);
  check(r == 0);

  r = epoll_wait(ephnd, evs, array_count(evs), 0);
  check(r == 0); /* No events should be reported. */

  check_dropped(ephnd, sock4);
  check_dropped(ephnd, sock5);

  r = epoll_close(ephnd);
  check(r == 0);

  return 0;
}
