/**
 * @file wepoll.h  like: `linux: sys/epoll.h`
 * @author your name (you@domain.com)
 * @brief wepoll - epoll for Windows
 * @version 0.1
 * @date 2025-02-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef _WEPOLL_H__
#define _WEPOLL_H__

#ifndef WEPOLL_EXPORT
#define WEPOLL_EXPORT
#endif

#include <stdint.h>

#ifndef __EPOLL_PACKED
# define __EPOLL_PACKED
#endif

enum EPOLL_EVENTS {
    EPOLLIN = (int)(1U << 0),
    EPOLLPRI = (int)(1U << 1),
    EPOLLOUT = (int)(1U << 2),
    EPOLLERR = (int)(1U << 3),
    EPOLLHUP = (int)(1U << 4),
    EPOLLRDNORM = (int)(1U << 6),
    EPOLLRDBAND = (int)(1U << 7),
    EPOLLWRNORM = (int)(1U << 8),
    EPOLLWRBAND = (int)(1U << 9),
    EPOLLMSG = (int)(1U << 10), /* Never reported. */
    EPOLLRDHUP = (int)(1U << 13),
    EPOLLONESHOT = (int)(1U << 31)
};

#define EPOLLIN (1U << 0)
#define EPOLLPRI (1U << 1)
#define EPOLLOUT (1U << 2)
#define EPOLLERR (1U << 3)
#define EPOLLHUP (1U << 4)
#define EPOLLRDNORM (1U << 6)
#define EPOLLRDBAND (1U << 7)
#define EPOLLWRNORM (1U << 8)
#define EPOLLWRBAND (1U << 9)
#define EPOLLMSG (1U << 10)
#define EPOLLRDHUP (1U << 13)
#define EPOLLONESHOT (1U << 31)

/* Valid opcodes ("op" parameter) to issue to `epoll_ctl()` */
#define EPOLL_CTL_ADD 1  /* Add a file descriptor to the interface. */
#define EPOLL_CTL_MOD 2  /* (3)Change file descriptor epoll_event structure.  */
#define EPOLL_CTL_DEL 3  /* (2)Remove a file descriptor from the interface. */

typedef void *HANDLE;
typedef uintptr_t SOCKET;

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
    SOCKET sock; /* Windows specific */
    HANDLE hnd; /* Windows specific */
} epoll_data_t;

struct epoll_event {
    uint32_t events;  /* Epoll events and flags */
    epoll_data_t data;  /* User data variable */
} __EPOLL_PACKED;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an epoll instance. 
 * 
 * @param size a hint specifying the number of file descriptors to be associated with the new instance. 
 * @return HANDLE Returns an `fd` for the new instance. The "fd" returned by epoll_create() should be closed with close().
 */
WEPOLL_EXPORT HANDLE epoll_create(int size);

/**
 * @brief Same as epoll_create, but with an FLAGS parameter.
 * The unused "SIZE" parameter has been dropped.
 * 
 * @param flags 
 * @return HANDLE fd 
 */
WEPOLL_EXPORT HANDLE epoll_create1(int flags);

WEPOLL_EXPORT int epoll_close(HANDLE ephnd);

/**
 * @brief Manipulate an epoll instance "ephnd"("efd").
 * 
 * @param ephnd 
 * @param op one of the EPOLL_CTL_* constants defined above.(opcode)
 * @param sock (fd) is the target of the operation
 * @param event describes which events the caller is interested in and any associated user data.
 * @return int Returns 0 in case of success, -1 in case of error (the "errno" variable will contain the specific error code) 
 */
WEPOLL_EXPORT int epoll_ctl(HANDLE ephnd, int op, SOCKET sock, struct epoll_event *event);

/**
 * @brief Wait for events on an epoll instance "ephnd"("efd").
 * 
 * @param ephnd 
 * @param events a buffer that will contain triggered events.
 * @param maxevents the maximum number of events to be returned (usually size of "events")
 * @param timeout specifies the maximum wait time in milliseconds (-1 == infinite).
 * @return int Returns the number of triggered events returned in "events" buffer. Or -1 in case of error with the "errno" variable set to the specific error code.
 */
WEPOLL_EXPORT int epoll_wait(HANDLE ephnd, struct epoll_event *events, int maxevents, int timeout);

#define array_count(a) (sizeof(a) / (sizeof((a)[0])))

WEPOLL_EXPORT int init(void);
WEPOLL_EXPORT int ws_global_init(void);

typedef struct reflock {
  volatile long state; /* 32-bit Interlocked APIs operate on `long` values. */
} reflock_t;
int reflock_global_init(void);
void reflock_init(reflock_t* reflock);
void reflock_ref(reflock_t* reflock);
void reflock_unref(reflock_t* reflock);
void reflock_unref_and_destroy(reflock_t* reflock);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* _WEPOLL_H__ */
