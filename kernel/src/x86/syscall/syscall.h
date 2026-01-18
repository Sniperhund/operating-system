#pragma once

#include "x86/idt.h"
#include <stdint.h>

extern "C" void syscallHandlerC(CPUStatus* s);

class Syscall {
public:
    static int init();

private:
    friend void ::syscallHandlerC(CPUStatus*);
    
    typedef uint32_t (*func) (CPUStatus*);

    static constexpr uint32_t MAX_SYSCALLS = 4;

    static func s_routines[MAX_SYSCALLS];

    static uint32_t read(CPUStatus* s);
    static uint32_t write(CPUStatus* s);
    static uint32_t open(CPUStatus* s);
    static uint32_t close(CPUStatus* s);
};