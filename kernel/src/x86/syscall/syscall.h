#pragma once

#include "x86/idt.h"
#include <stdint.h>

extern "C" void syscallHandlerC(CPUStatus* s);

class Syscall {
public:
    static int init();

private:
    friend void ::syscallHandlerC(CPUStatus*);

    typedef void (*func) (CPUStatus*);

    static constexpr uint32_t MAX_SYSCALLS = 4;

    static func s_routines[MAX_SYSCALLS];

    static void read(CPUStatus* s);
    static void write(CPUStatus* s);
    static void open(CPUStatus* s);
    static void close(CPUStatus* s);
};