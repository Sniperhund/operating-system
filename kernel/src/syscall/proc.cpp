#include "syscall.h"

#include "sched/scheduler.h"
#include "stdio.h"

uint32_t Syscall::exit(CPUStatus* s) {
    current->exitCode = s->eax;
    current->state = EXITED;
    Scheduler::switchTask(s);

    return 0;
}