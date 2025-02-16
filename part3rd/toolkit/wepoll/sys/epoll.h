/**
 * @file epoll.h
 * @author your name (you@domain.com)
 * @brief epoll for my toolkit
 * @version 0.1
 * @date 2025-02-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef _EPOLL_H__
#define _EPOLL_H__
#include "wepoll.h"
#include <map>
#include <mutex>

// 屏蔽 EPOLLET
#define EPOLLET 0

namespace toolkit {
    // 索引handle
    extern std::map<int, HANDLE> s_wepollHandleMap;
    extern int s_handleIndex;
    extern std::mutex s_handleMtx;
    
    // 屏蔽epoll_create epoll_ctl epoll_wait参数差异
    inline int epoll_create(int size) {
        HANDLE handle = ::epoll_create(size);
        if (!handle) {
            return -1;
        }
        {
            std::lock_guard<std::mutex> lck(s_handleMtx);
            int idx = ++s_handleIndex;
            s_wepollHandleMap[idx] = handle;
            return idx;
        }
    }

    inline int epoll_ctl(int ephnd, int op, SOCKET sock, struct epoll_event *ev) {
        HANDLE handle;
        {
            std::lock_guard<std::mutex> lck(s_handleMtx);
            handle = s_wepollHandleMap[ephnd];
        }
        return ::epoll_ctl(handle, op, sock, ev);
    }

    inline int epoll_wait(int ephnd, struct epoll_event *events, int maxevents, int timeout) {
        HANDLE handle;
        {
            std::lock_guard<std::mutex> lck(s_handleMtx);
            handle = s_wepollHandleMap[ephnd];
        }
        return ::epoll_wait(handle, events, maxevents, timeout);
    }
} // namespace toolkit

#endif /* _EPOLL_H__ */
