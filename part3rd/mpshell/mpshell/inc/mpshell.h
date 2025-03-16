#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <time.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "types.h"

#define DEFAULT_SLEEP  100  // 100ms
#define DEFAULT_TIMEOUT 2   // 2s

#define MPSHELL_EXPORT 

MPSHELL_EXPORT void init_buffers(void);
MPSHELL_EXPORT void cleanup(void);
MPSHELL_EXPORT void show_usage(bool windows);

MPSHELL_EXPORT void prepare_ok_response(void);
MPSHELL_EXPORT void prepare_error_response(void);

MPSHELL_EXPORT void execute_command(const char *command);

MPSHELL_EXPORT void process_packet(
    ssize_t num_bytes_received,
    mp_payload_header *header_in,
    mp_payload_header *header_out,
    unsigned char *data_in,
    unsigned char *data_out
);

MPSHELL_EXPORT int open_tcp_channel(const char *host, const char *port);
MPSHELL_EXPORT int open_udp_channel(const char *host, int port);
MPSHELL_EXPORT int open_icmp_channel(const char *host);
