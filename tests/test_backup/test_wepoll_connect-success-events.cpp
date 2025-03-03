/**
 * Check that stream sockets connected with `connect()` do not continuously report EPOLLOUT, but only when they're actually writable.
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

// ###############################################################
#define array_count(a) (sizeof(a) / (sizeof((a)[0])))
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "wepoll.h"

#define PORT 12345
#define ROUNDS 5
#define SEND_LENGTH (1 << 20)

int main(void) {
    printf("test connect success events\n");

    unsigned long one = 1;
    SOCKET listen_sock, server_sock, client_sock;
    struct sockaddr_in addr;
    HANDLE ephnd;
    struct epoll_event ev;
    int round;
    int r;

    /* Initialize winsock. */
    r = ws_global_init();
    check(r == 0);

    /* Set up address that the server listens on and the client connects to. */
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(PORT);

    /* Create the server socket and start listening for incoming connections. */
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    check(listen_sock != INVALID_SOCKET);
    r = bind(listen_sock, (struct sockaddr*)&addr, sizeof addr);
    check(r == 0);
    r = listen(listen_sock, SOMAXCONN);
    check(r == 0);

    /* Create the client-end socket, switch it to non-blocking mode. */
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    check(client_sock != INVALID_SOCKET);
    r = ioctlsocket(client_sock, (long)FIONBIO, &one);
    check(r == 0);

    /* Create an epoll port for the client end of the connection. */
    ephnd = epoll_create1(0);
    check(ephnd != NULL);

    /* Associate the client socket with the epoll port. */
    ev.events = EPOLLOUT;
    ev.data.sock = client_sock;
    r = epoll_ctl(ephnd, EPOLL_CTL_ADD, client_sock, &ev);
    check(r == 0);

    /* Begin connecting the client socket to the server. */
    r = connect(client_sock, (struct sockaddr*)&addr, sizeof addr);
    check(r == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK);

    /* Accept the server end of the connection. */
    server_sock = accept(listen_sock, NULL, NULL);
    check(server_sock != INVALID_SOCKET);

    for (round = 0; round < ROUNDS; round++) {
        static char buf[SEND_LENGTH];
        struct epoll_event evs[8];
        int bytes_received, bytes_sent;
        /* Wait for client end to become writable. */
        r = epoll_wait(ephnd, evs, array_count(evs), -1);
        /* Try to send data until the kernel buffer is full. */
        memset(buf, round, sizeof buf);
        bytes_sent = 0;
        do {
            /* We shouldn't get here unless epoll_wait() reported that the client socket is writable. */
            check(r == 1);
            check(evs[0].data.sock == client_sock);
            check(evs[0].events == EPOLLOUT);
            /* The actual send() call should never fail, because epoll_wait() just reported the socket as writable. */
            r = send(client_sock, buf, sizeof buf, 0);
            check(r > 0);
            bytes_sent += r;
            /* Call epoll_wait() without blocking to see if there's more space in the kernel write buffer. */
            r = epoll_wait(ephnd, evs, array_count(evs), 0);
        } while (r > 0);

        /* Verify that epoll_wait() reported no events, but did not fail. */
        check(r == 0);
        /* Read all data incoming on the server end of the connection. */
        bytes_received = 0;
        do {
            r = recv(server_sock, buf, sizeof buf, 0);
            check(r > 0);
            check(buf[0] == round);
            bytes_received += r;
        } while (bytes_received < bytes_sent);
        check(bytes_received == bytes_sent);
    }

    /* Disassociate the client socket from epoll port. */
    r = epoll_ctl(ephnd, EPOLL_CTL_DEL, client_sock, NULL);
    check(r == 0);

    /* Close the epoll port. */
    r = epoll_close(ephnd);
    check(r == 0);

    /* Close all sockets. */
    r = closesocket(listen_sock);
    check(r == 0);
    r = closesocket(server_sock);
    check(r == 0);
    r = closesocket(client_sock);
    check(r == 0);

    return 0;
}