#include "process.h"
#include "exec/pid.h"
#include "fs/vfs.h"
#include "panic.h"
#include "sched/scheduler.h"
#include "x86/memory/heap.h"
#include "x86/memory/paging.h"
#include "string.h"
#include "error.h"

int Proc::addFd(inode* file) {
    for (size_t fd = 0; fd < MAX_FDS; fd++) {
        if (!files.fds[fd]) {
            files.fds[fd] = file;
            if (fd > files.maxFd) files.maxFd = fd;
            return fd;
        }
    }

    return -E_MFILE;
}

int Proc::removeFd(size_t fd) {
    if (files.maxFd >= fd || !files.fds[fd]) return -E_MFILE;

    files.fds[fd] = nullptr;
    if (fd == files.maxFd) {
        while (files.maxFd > 0 && files.fds[files.maxFd] == nullptr)
            files.maxFd--;
    }

    return 0;
}

const char* Proc::stateString() {
    switch (state) {
        case RUNNING: return "RUNNING";
        case READY: return "READY";
        case BLOCKED: return "BLOCKED";
        case NEW: return "NEW";
        case KILLED: return "KILLED";
        case EXITED: return "EXITED";
    }

    return "UNDEFINED";
}

Proc* Proc::createProcess() {
    Proc* proc = (Proc*)Heap::alloc(sizeof(Proc));
    memset(proc, 0, sizeof(Proc));
    proc->exitCode = 255;
    proc->state = NEW;
    proc->pid = PID::allocate();
    proc->pd = Paging::createPD();

    inode* stdin = VFS::open("/dev/stdin", O_READ);
    if (!stdin) return (Proc*)-E_PROC;

    proc->files.fds[0] = stdin;

    inode* stdout = VFS::open("/dev/stdout", O_WRITE);
    if (!stdout) return (Proc*)-E_PROC;

    proc->files.fds[1] = stdout;

    inode* stderr = VFS::open("/dev/stderr", O_WRITE);
    if (!stderr) return (Proc*)-E_PROC;

    proc->files.fds[2] = stderr;

    if (!proc->pd) return (Proc*)-E_NOMEM;

    return proc;
}

void Proc::freeProcess(Proc *proc) {
    if (proc == current) PANIC("Process", "Attempted to free active process");

    Paging::freePD(proc->pd);
    Heap::free(proc->kstack);
    // NOTE: Really inefficient, but it will most likely cause an exception if this proc is used again
    //       And is therefore easier to catch bugs
    memset(proc, 0, sizeof(Proc));
    Heap::free(proc);
}