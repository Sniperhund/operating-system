#include "syscall.h"
#include "x86/idt.h"
#include "panic.h"
#include "string.h"
#include "stdio.h"

Syscall::func Syscall::s_routines[Syscall::MAX_SYSCALLS];

int Syscall::init() {
    memset(&s_routines, 0, sizeof(func) * MAX_SYSCALLS);

    s_routines[0] = read;
    s_routines[1] = write;
    s_routines[2] = open;
    s_routines[3] = close;

    return 0;
}

extern "C" void syscallHandlerC(CPUStatus* s) {
    printf("Syscall! %d, %d, 0x%p", s->eax, Syscall::MAX_SYSCALLS, Syscall::s_routines[s->eax]);

    if (s->eax < Syscall::MAX_SYSCALLS && Syscall::s_routines[s->eax]) {
        Syscall::s_routines[s->eax](s);

        return;
    }

    PANIC("Syscall", "Syscall not found");
}