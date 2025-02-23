/**
 * Public header file in `trantor` lib
 */
#ifndef _NONCOPYABLE_H__
#define _NONCOPYABLE_H__

#include "exports.h"

namespace trantor {
    class TRANTOR_EXPORT NonCopyable {
    protected:
        NonCopyable() {}
        ~NonCopyable() {}
        
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable &operator=(const NonCopyable &) = delete;

        // some uncopyable classes maybe support move constructor...
        NonCopyable(NonCopyable &&) noexcept(true) = default;
        NonCopyable &operator=(NonCopyable &&) noexcept(true) = default;
    };
}  // namespace trantor

#endif 