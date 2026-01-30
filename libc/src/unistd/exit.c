#include "syscall.h"
#include <unistd.h>

int exit(int status) {
    SYSCALL1(0x04, status);
    return 0;
}