#pragma once

#include "fs/vfs.h"
#include "stdint.h"
#include "stddef.h"
#include <sys/types.h>

enum State {
    RUNNING,
    READY,
    BLOCKED,
    NEW,
    KILLED,
    EXITED
};

struct CPUContext {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp, eip;
    uint32_t eflags;
    uint32_t useresp, ss;
} __attribute__((packed));

#define MAX_FDS 32

struct Files {
    inode* fds[MAX_FDS];
    uint32_t maxFd;
};

struct Proc {
    pid_t pid;
    pid_t ppid;

    State state;
    CPUContext ctx;
    Files files;

    void* stack;
    void* kstack;
    void* pd;

    int exitCode;

    int addFd(inode* file);
    int removeFd(size_t fd);

    static Proc* createProcess();
    static void freeProcess(Proc* proc);
};

void exec(const char* cmd, const char* args);