#include "syscall.h"
#include "sched/scheduler.h"
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
    
    s_routines[4] = exit;
    s_routines[5] = getpid;
    s_routines[6] = getppid;
    s_routines[7] = exec;
    s_routines[8] = fork;

    return 0;
}

extern "C" void syscallHandlerC(CPUStatus* s) {
    if (s->eax >= Syscall::MAX_SYSCALLS || !Syscall::s_routines[s->eax]) {
        PANIC("Syscall", "Syscall not found");
    }

    int ret = Syscall::s_routines[s->eax](s);

    // Return value to userspace
    s->eax = ret;
}