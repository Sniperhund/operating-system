#include "error.h"
#include "fs/vfs.h"
#include "sched/scheduler.h"
#include "syscall.h"
#include "stdio.h"

int Syscall::read(CPUStatus* s) {

}

int Syscall::write(CPUStatus* s) {

}

int Syscall::open(CPUStatus* s) {
    inode* file = VFS::open((const char*)s->ebx, (uint32_t)s->ecx);
    if (!file) {
        return -E_NOENT;
    }

    int fd = current->addFd(file);
    if (fd == -1) {
        return -E_MFILE;
    }

    printf("Returning fd: %d\n", fd);

    return fd;
}

int Syscall::close(CPUStatus* s) {
}
