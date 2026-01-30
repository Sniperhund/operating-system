#include "stdio.h"
#include "syscall.h"

int fread(int fd, char* buf, size_t count, size_t offset) {
    int ret = SYSCALL4(0x00, fd, buf, count, offset);
    return __syscall_ret(ret);
}

int fwrite(int fd, const char* buf, size_t count, size_t offset) {
    int ret = SYSCALL4(0x01, fd, buf, count, offset);
    return __syscall_ret(ret);
}

int fopen(const char* path, uint32_t mode) {
    int ret = SYSCALL2(0x02, path, mode);
    return __syscall_ret(ret);
}

int fclose(int fd) {
    int ret = SYSCALL1(0x03, fd);
    return __syscall_ret(ret);
}