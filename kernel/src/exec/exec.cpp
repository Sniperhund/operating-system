#include "process.h"
#include "debug.h"
#include "exec/elfloader.h"
#include "fs/vfs.h"
#include "panic.h"
#include "sched/scheduler.h"
#include "string.h"
#include "x86/memory/heap.h"
#include "x86/memory/paging.h"
#include "error.h"
#include "stdio.h"

// TODO: Move these away
#define KERNEL_STACK_SIZE   1024 * 16 // 16 KiB
#define USER_STACK_SPACE    0xB0000000
#define USER_STACK_SIZE     1024 * 32 // 32 KiB
#define MAX_ARGS            32

int splitArgs(const char* args, const char** argv, int max) {
    int argc = 0;

    while (*args && argc < max) {
        while (*args == ' ') args++;
        if (!*args) break;

        argv[argc++] = args;

        while (*args && *args != ' ') args++;
        if (*args) *((char*)args++) = 0;
    }

    return argc;
}

void setupArgs(Proc* proc, const char* args) {
    // Give args to program.
    uintptr_t sp = (uintptr_t)proc->stack;

    char* argBuffer = strdup(args ? args : "");
    const char* argvTemp[MAX_ARGS] = {0};
    int argc = splitArgs(argBuffer, argvTemp, MAX_ARGS);

    uintptr_t argvPtrs[MAX_ARGS] = {0};

    for (int i = argc - 1; i >= 0; i--) {
        int len = strlen(argvTemp[i]) + 1;

        sp -= len;
        sp &= ~0xF;
        memcpy((void*)sp, argvTemp[i], len);

        argvPtrs[i] = sp;
    }

    sp &= ~0xF;
    sp -= sizeof(uintptr_t);
    *(uintptr_t*)sp = 0;

    for (int i = argc - 1; i >= 0; i--) {
        sp -= sizeof(uintptr_t);
        *(uintptr_t*)sp = argvPtrs[i];
    }
    
    sp -= sizeof(uintptr_t);
    *(uintptr_t*)sp = argc;
    proc->ctx.useresp = (uintptr_t)sp;
}

void exec(const char *cmd, const char *args) {
    Proc* proc = current;

    if (proc == nullptr) PANIC("PROCESS", "Can't use exec with no current process");

    const char* cmdC = strdup(cmd);
    const char* argsC = strdup(args);

    void* oldPD = proc->pd;
    proc->pd = Paging::createPD();

    void* userStackBottom = mmap(proc->pd, (void*)USER_STACK_SPACE, USER_STACK_SIZE, PROT_WRITE);
    proc->stack = (void*)((uintptr_t)userStackBottom + USER_STACK_SIZE);

    Paging::switchPD(proc->pd, false);
    Paging::freePD(oldPD);

    setupArgs(proc, argsC);

    inode* file = VFS::open(cmdC);
    if (!file) PANIC("PROCESS", "Couldn't open file");

    void* buffer = Heap::alloc(file->size);
    VFS::read(file, buffer, 0, file->size);

    ELFLoader::loadExecutable(buffer, &proc->ctx.eip);
    loadDebugSymbols(cmdC, proc->ctx.eip);
    
    Heap::free(buffer);

    proc->state = READY;
    proc->ctx.eflags = 0x202;

    Scheduler::switchTo(proc);
}

void spawn(const char *cmd, const char *args) {
    // Start by getting a process, that has state to "NEW"
    Proc* proc = Proc::createProcess();

    if ((intptr_t)proc == -E_NOMEM) PANIC("PROCESS", "Page Directory couldn't be allocated");
    if ((intptr_t)proc == -E_PROC) PANIC("PROCESS", "Couldn't open stdout or stderr");

    proc->kstack = (void*)((uintptr_t)Heap::alloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE);

    void* userStackBottom = mmap(proc->pd, (void*)USER_STACK_SPACE, USER_STACK_SIZE + PAGE_SIZE, PROT_WRITE);
    proc->stack = (void*)((uintptr_t)userStackBottom + USER_STACK_SIZE);

    Paging::switchPD(proc->pd, false);

    setupArgs(proc, args);
    
    proc->ctx.esp = (uintptr_t)proc->kstack;

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