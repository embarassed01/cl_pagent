/**
 * @file epoll.cpp
 * @author your name (you@domain.com)
 * @brief epoll for my toolkit
 * @version 0.1
 * @date 2025-02-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "epoll.h"

std::map<int, HANDLE> toolkit::s_wepollHandleMap;
int toolkit::s_handleIndex = 0;
std::mutex toolkit::s_handleMtx;
