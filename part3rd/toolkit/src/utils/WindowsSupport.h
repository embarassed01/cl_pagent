#ifndef _WINDOWS_SUPPORT_H_TLKIT__
#define _WINDOWS_SUPPORT_H_TLKIT__

#include "exports.h"
#include <stdint.h>

struct iovec {
    void *iov_base;  // Starting address
    int iov_len;  // Number of bytes
};

TRANTOR_EXPORT int readv(int fd, const struct iovec *vector, int count);

#endif 