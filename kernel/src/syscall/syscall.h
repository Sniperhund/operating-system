#pragma once

#include "x86/idt.h"
#include <stdint.h>

extern "C" void syscallHandlerC(CPUStatus* s);

class Syscall {
public:
    static int init();

    static CPUStatus* s_currentStatus;

private:
    friend void ::syscallHandlerC(CPUStatus*);
    
    typedef int (*func) (CPUStatus*);

    static constexpr uint32_t MAX_SYSCALLS = 16;

    static func s_routines[MAX_SYSCALLS];

    // File
    static int read(CPUStatus* s);
    static int write(CPUStatus* s);
    static int open(CPUStatus* s);
    static int close(CPUStatus* s);

    // Process
    static int exit(CPUStatus* s);
    static int getpid(CPUStatus* s);
    static int getppid(CPUStatus* s);
    static int exec(CPUStatus* s);
    static int fork(CPUStatus* s);
};
