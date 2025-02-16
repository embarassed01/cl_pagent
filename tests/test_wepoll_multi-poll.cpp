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
#include <process.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "wepoll.h"

#define PORT_COUNT 10
#define THREADS_PER_PORT 10

#define LISTEN_PORT 12345

typedef struct test_context {
    SOCKET sock;
    HANDLE port;
    uint64_t data;
    HANDLE thread;
} test_context_t;

static SOCKET create_socket(unsigned short port) {
    SOCKET sock;
    struct sockaddr_in address;
    unsigned long one = 1;
    int r;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    check(sock > 0);

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    r = bind(sock, (struct sockaddr*)&address, sizeof address);
    check(r == 0);

    r = ioctlsocket(sock, (long)FIONBIO, &one);
    check(r == 0);

    return sock;
}

static void send_message(SOCKET sock, unsigned short port) {
    char hello[] = "hello";
    struct sockaddr_in address;
    WSABUF buf;
    DWORD bytes_sent;
    int r;

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    buf.buf = hello;
    buf.len = sizeof(hello);

    r = WSASendTo(sock, &buf, 1, &bytes_sent, 0, (struct sockaddr*)&address, sizeof address, NULL, NULL);
    check(r >= 0);
}

static unsigned int __stdcall poll_thread(void *arg) {
    test_context_t *context = (test_context_t*)arg;
    struct epoll_event ev_out;
    int r;

    memset(&ev_out, 0, sizeof ev_out);
    r = epoll_wait(context->port, &ev_out, 1, -1);
    check(r == 1);

    check(ev_out.events == EPOLLIN);
    check(ev_out.data.u64 == context->data);

    printf("Got event(port %p, thread %p)\n", context->port, context->thread);

    return 0;
}

int main(void) {
    printf("test multi poll\n");

    HANDLE ports[PORT_COUNT];
    test_context_t contexts[PORT_COUNT][THREADS_PER_PORT];
    WSADATA wsa_data;
    size_t i, j;
    int r;

    /* Initialize winsock. */
    r = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    check(r == 0);

    SOCKET send_sock = create_socket(0);
    SOCKET recv_sock = create_socket(LISTEN_PORT);

    /* Create PORT_COUNT epoll ports which, will be polled by THREADS_PER_PORT threads. */
    for (i = 0; i < array_count(contexts); i++) {
        HANDLE port;
        struct epoll_event ev;

        /* Create epoll port. */
        port = epoll_create1(0);
        check(port != NULL);
        ports[i] = port;

        /* Register recv_sock with the epoll port. */
        ev.events = EPOLLIN;
        ev.data.u64 = (uint64_t)rand();
        r = epoll_ctl(port, EPOLL_CTL_ADD, recv_sock, &ev);
        check(r == 0);

        /* Start THREADS_PER_PORT threads which will all poll the port. */
        for (j = 0; j < array_count(contexts[i]); j++) {
            test_context_t *context = &contexts[i][j];
            HANDLE thread;
            /* Prepare context information for the polling thread. */
            context->port = port;
            context->sock = recv_sock;
            context->data = ev.data.u64;
            /* Start thread. */
            thread = (HANDLE)_beginthreadex(NULL, 0, poll_thread, (void*)context, 0, NULL);  // typedef unsigned (__stdcall *_beginthreadex_proc_type)(void *);
            check(thread != INVALID_HANDLE_VALUE);
            context->thread = thread;
        }
    }

    /* Sleep for a while to give all threads a chance to finish initializing. */
    Sleep(500);

    /* Send a message to the receiving socket. */
    send_message(send_sock, LISTEN_PORT);

    /* Wait for all threads to exit and clean up after them. */
    for (i = 0; i < array_count(contexts); i++) {
        for (j = 0; j < array_count(contexts[i]); j++) {
            HANDLE thread = contexts[i][j].thread;
            DWORD wr = WaitForSingleObject(thread, INFINITE);
            check(wr == WAIT_OBJECT_0);
            CloseHandle(thread);
        }
    }

    /* Close all epoll ports. */
    for (i = 0; i < array_count(ports); i++) {
        HANDLE port = ports[i];
        r = epoll_close(port);
        check(r == 0);
    }

    return 0;
}