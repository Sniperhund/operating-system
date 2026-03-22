#include "syscall.h"
#include <unistd.h>

int exit(int status) {
    SYSCALL1(4, status);
    return 0;
}