#include "syscall.h"
#include "sched/scheduler.h"
#include "x86/idt.h"
#include "panic.h"
#include "string.h"
#include "stdio.h"

Syscall::func Syscall::s_routines[Syscall::MAX_SYSCALLS];

int Syscall::init() {
    memset(&s_routines, 0, sizeof(func) * MAX_SYSCALLS);

    s_routines[0x0] = read;
    s_routines[0x1] = write;
    s_routines[0x2] = open;
    s_routines[0x3] = close;
    
    s_routines[0x4] = exit;

    return 0;
}

extern "C" void syscallHandlerC(CPUStatus* s) {
    if (s->eax >= Syscall::MAX_SYSCALLS || !Syscall::s_routines[s->eax]) {
        PANIC("Syscall", "Syscall not found");
    }

    printf("Syscall %d called\n", s->eax);

    int ret = Syscall::s_routines[s->eax](s);

    // Return value to userspace
    s->eax = ret;
}