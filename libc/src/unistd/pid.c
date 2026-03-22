#include "syscall.h"
#include "unistd.h"

pid_t getpid() {
    int ret = SYSCALL0(5);
    return __syscall_ret(ret);
}

pid_t getppid() {
    int ret = SYSCALL0(6);
    return __syscall_ret(ret);
}