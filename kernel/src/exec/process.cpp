#include "process.h"
#include "debug.h"
#include "exec/elfloader.h"
#include "exec/pid.h"
#include "fs/vfs.h"
#include "panic.h"
#include "sched/scheduler.h"
#include "x86/memory/heap.h"
#include "x86/memory/paging.h"
#include "string.h"
#include "error.h"

// TODO: Move these away
#define KERNEL_STACK_SIZE   1024 * 16 // 16 KiB
#define USER_STACK_SPACE    0xB0000000
#define USER_STACK_SIZE     1024 * 32 // 32 KiB

int Proc::addFd(inode* file) {
    for (size_t fd = 0; fd < MAX_FDS; fd++) {
        if (!files.fds[fd]) {
            files.fds[fd] = file;
            if (fd > files.maxFd) files.maxFd = fd;
            return fd;
        }
    }

    return -1;
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

    inode* stdout = VFS::open("/dev/stdout", 0);
    if (!stdout) return (Proc*)-E_PROC;

    proc->files.fds[0] = stdout;

    inode* stderr = VFS::open("/dev/stderr", 0);
    if (!stderr) return (Proc*)-E_PROC;

    proc->files.fds[1] = stderr;

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

void exec(const char *cmd, const char *args) {
    // Start by getting a process, that has state to "NEW"
    Proc* proc = Proc::createProcess();

    if ((intptr_t)proc == -E_NOMEM) PANIC("PROCESS", "Page Directory couldn't be allocated");
    if ((intptr_t)proc == -E_PROC) PANIC("PROCESS", "Couldn't open stdout or stderr");

    proc->kstack = (void*)((uintptr_t)Heap::alloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE);

    void* userStackBottom = mmap(proc->pd, (void*)USER_STACK_SPACE, USER_STACK_SIZE + PAGE_SIZE, PROT_WRITE);
    proc->stack = (void*)((uintptr_t)userStackBottom + USER_STACK_SIZE);
    proc->ctx.useresp = (uintptr_t)proc->stack;
    proc->ctx.esp = (uintptr_t)proc->kstack;


    Paging::switchPD(proc->pd, false);

    inode* file = VFS::open(cmd);
    if (!file) PANIC("PROCESS", "Couldn't open file");

    void* buffer = Heap::alloc(file->size);
    VFS::read(file, buffer, 0, file->size);

    ELFLoader::loadExecutable(buffer, &proc->ctx.eip);
    loadDebugSymbols(cmd, proc->ctx.eip);

    Heap::free(buffer);

    proc->state = READY;
    proc->ctx.eflags = 0x202;

    asm volatile("cli");
    Scheduler::addProcess(proc);
    asm volatile("sti");
}