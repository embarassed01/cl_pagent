#include "tlshell/inc/header.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    return client(argv[1], atoi(argv[2]));
}