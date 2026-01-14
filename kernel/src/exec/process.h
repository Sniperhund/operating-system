#pragma once

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

struct Proc {
    uint32_t pid;
    uint32_t ppid;

    State state;
    CPUContext ctx;

    void* stack;
    void* kstack;
    void* pd;

    int exitCode;

    static Proc* createProcess();
};

void exec(const char* cmd, const char* args);