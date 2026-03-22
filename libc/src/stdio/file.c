#include "stdio.h"
#include "syscall.h"

int fread(int fd, char* buf, size_t count, size_t offset) {
    int ret = SYSCALL4(0, fd, buf, count, offset);
    return __syscall_ret(ret);
}

int fwrite(int fd, const char* buf, size_t count, size_t offset) {
    int ret = SYSCALL4(1, fd, buf, count, offset);
    return __syscall_ret(ret);
}

int fopen(const char* path, uint32_t mode) {
    int ret = SYSCALL2(2, path, mode);
    return __syscall_ret(ret);
}

int fclose(int fd) {
    int ret = SYSCALL1(3, fd);
    return __syscall_ret(ret);
}