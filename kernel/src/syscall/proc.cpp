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
