#include "error.h"
#include "fs/vfs.h"
#include "sched/scheduler.h"
#include "syscall.h"
#include "stdio.h"

int Syscall::read(CPUStatus* s) {
    if (s->ebx > MAX_FDS) return -E_INVAL;

    inode* file = current->files.fds[s->ebx];
    if (!file) return -E_NOENT;

    return VFS::read(file, (void*)s->ecx, s->esi, s->edx);
}

int Syscall::write(CPUStatus* s) {
    if (s->ebx > MAX_FDS) return -E_INVAL;

    inode* file = current->files.fds[s->ebx];
    if (!file) return -E_NOENT;

    return VFS::write(file, (void*)s->ecx, s->esi, s->edx);
}

int Syscall::open(CPUStatus* s) {
    inode* file = VFS::open((const char*)s->ebx, (uint32_t)s->ecx);
    if (!file) return -E_NOENT;

    int fd = current->addFd(file);
    if (fd == -1) return -E_MFILE;

    printf("Returning fd: %d\n", fd);

    return fd;
}

int Syscall::close(CPUStatus* s) {
    if (s->ebx > MAX_FDS) return -E_INVAL;

    inode* file = current->files.fds[s->ebx];
    if (!file) return -E_NOENT;

    VFS::close(file);

    return 0;

}
