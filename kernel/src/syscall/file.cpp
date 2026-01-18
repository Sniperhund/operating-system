#include "error.h"
#include "fs/vfs.h"
#include "sched/scheduler.h"
#include "syscall.h"
#include "stdio.h"

uint32_t Syscall::read(CPUStatus* s) {

}

uint32_t Syscall::write(CPUStatus* s) {

}

uint32_t Syscall::open(CPUStatus* s) {
    inode* file = VFS::open((const char*)s->ebx, (uint32_t)s->ecx);
    if (!file) {
        return E_NOENT;
    }

    int fd = current->addFd(file);
    if (fd == -1) {
        return E_MFILE;
    }

    return fd;
}

uint32_t Syscall::close(CPUStatus* s) {
}
