#include "exec/process.h"
#include "syscall.h"
#include "sched/scheduler.h"
#include <stdio.h>

int Syscall::exit(CPUStatus* s) {
    current->exitCode = s->ebx;
    current->state = EXITED;

    printf("Process exited with: %d\n", current->exitCode);

    Scheduler::switchTask(s);

    return 0;
}

int Syscall::getpid(CPUStatus* s) {
    return current->pid;
}

int Syscall::getppid(CPUStatus* s) {
    return current->ppid;
}

int Syscall::exec(CPUStatus* s) {
    ::exec((const char*)s->ebx, (const char*)s->ecx);

    return 0;
}

int Syscall::fork(CPUStatus* s) {
    return 0;
}