#pragma once

#include "fs/vfs.h"
#include "stdint.h"
#include "stddef.h"

enum State {
    RUNNING,
    READY,
    BLOCKED,
    NEW,
    KILLED
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
    uint32_t pid;
    uint32_t ppid;

    State state;
    CPUContext ctx;
    Files files;

    void* stack;
    void* kstack;
    void* pd;

    int exitCode;

    uint32_t errorNo;

    int addFd(inode* file);
    int removeFd(size_t fd);

    static Proc* createProcess();
};

void exec(const char* cmd, const char* args);