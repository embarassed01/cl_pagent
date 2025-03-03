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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "wepoll.h"

#define THREAD_COUNT 20
#define TEST_ITERATIONS 50
#define TEST_LENGTH 100

typedef struct test_context {
    reflock_t reflock;
    SRWLOCK srwlock;
    volatile bool stop;
} test_context_t;

static void init_context(test_context_t *context) {
    reflock_init(&context->reflock);
    InitializeSRWLock(&context->srwlock);
    context->stop = false;
}

static void yield(void) {
    int count = rand() % 3;
    while (count--) Sleep(0);
}

static unsigned int __stdcall test_thread(void *arg) {
    test_context_t *context = (test_context_t*)arg;
    uint64_t lock_count = 0;
    AcquireSRWLockShared(&context->srwlock);
    while (!context->stop) {
        reflock_ref(&context->reflock);
        ReleaseSRWLockShared(&context->srwlock);
        
        lock_count++;
        yield();
        
        reflock_unref(&context->reflock);
        yield();

        AcquireSRWLockShared(&context->srwlock);
    }
    ReleaseSRWLockShared(&context->srwlock);
    check(lock_count > 100);  // Hopefully much more.
    return 0;
}

static void destroy_reflock(test_context_t *context) {
    AcquireSRWLockExclusive(&context->srwlock);
    context->stop = true;
    reflock_ref(&context->reflock);
    ReleaseSRWLockExclusive(&context->srwlock);
    reflock_unref_and_destroy(&context->reflock);
}

static void run_test_iteration(void) {
    test_context_t context;
    HANDLE threads[THREAD_COUNT];
    size_t i;

    init_context(&context);
    for (i = 0; i < array_count(threads); i++) {
        HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, test_thread, &context, 0, NULL);
        check(thread != INVALID_HANDLE_VALUE);
        threads[i] = thread;
    }
    Sleep(TEST_LENGTH);
    destroy_reflock(&context);
    for (i = 0; i < array_count(threads); i++) {
        HANDLE thread = threads[i];
        DWORD wr = WaitForSingleObject(thread, INFINITE);
        check(wr == WAIT_OBJECT_0);
        CloseHandle(thread);
    }
}

int main(void) {
    printf("test reflock\n");

    int i;
    if (init() < 0) return 0;
    
    for (i = 0; i < TEST_ITERATIONS; i++) {
        printf("Iteration %d of %d\n", i + 1, TEST_ITERATIONS);
        run_test_iteration();
    }
    return 0;
}