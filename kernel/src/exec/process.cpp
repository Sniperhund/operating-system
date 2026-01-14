#include "process.h"
#include "exec/elfloader.h"
#include "fs/vfs.h"
#include "panic.h"
#include "sched/scheduler.h"
#include "x86/memory/heap.h"
#include "x86/memory/paging.h"
#include "string.h"

// TODO: Move these away
#define KERNEL_STACK_SIZE   1024 * 16 // 16 KiB
#define USER_STACK_SPACE    0xB0000000
#define USER_STACK_SIZE     1024 * 32 // 32 KiB

Proc* Proc::createProcess() {
    Proc* proc = (Proc*)Heap::alloc(sizeof(Proc));
    memset(proc, 0, sizeof(Proc));
    proc->exitCode = 255;
    proc->state = NEW;
    proc->pid = 0; // TODO: Add system to track these
    proc->pd = Paging::createPD();

    if (!proc->pd) return nullptr;

    return proc;
}

void exec(const char *cmd, const char *args) {
    // Start by getting a process, that has state to "NEW"
    Proc* proc = Proc::createProcess();

    if (!proc->pd) PANIC("PROCESS", "Page Directory couldn't be allocated");

    // Create a kernel and user stack

    // Heap allocator allocates in higher half, so is always mapped
    proc->kstack = (void*)((uintptr_t)Heap::alloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE);
    // mmap makes it possible to map and "allocate" in lower half
    void* userStackBottom = mmap(proc->pd, (void*)USER_STACK_SPACE, USER_STACK_SIZE, PROT_WRITE);
    proc->stack = (void*)((uintptr_t)userStackBottom + USER_STACK_SIZE);
    proc->ctx.useresp = (uintptr_t)proc->stack;
    proc->ctx.esp = (uintptr_t)proc->kstack;

    // Switch to the page directory given (kernel is automatically mapped)
    Paging::switchPD(proc->pd, false);
    // Load the ELF (maps user pages itself)
    inode* file = VFS::open(cmd);
    if (!file) PANIC("PROCESS", "Couldn't open file");

    void* buffer = Heap::alloc(file->size);
    VFS::read(file, buffer, 0, file->size);

    ELFLoader::loadExecutable(buffer, &proc->ctx.eip);

    Heap::free(buffer);
    // Set state to READY
    proc->state = READY;
    proc->ctx.eflags = 0x202;
    // Add the process to the scheduler
    asm volatile("cli");
    Scheduler::addProcess(proc);
    asm volatile("sti");
    // Notify scheduler that it's ready, when context switching is implemented let the scheduler do that itself
}