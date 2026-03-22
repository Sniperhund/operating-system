#pragma once

#include "exec/process.h"
#include "x86/idt.h"

extern Proc* current;

class Scheduler {
public:
    static int init();
    static void addProcess(Proc* proc);
    
    __attribute__((noreturn))
    static void run();

    static void switchTask(CPUStatus* cpu);

    static Proc* getByPid(pid_t pid);

    /**
     * Force a switch to another process.
     * This doesn't save the previous process only switches, therefore it should only be used if the previous process doesn't matter.
     */
    static void switchTo(Proc* next);

private:
    static constexpr uint32_t MAX_PROCESSES = 32;

    static Proc* s_processes[MAX_PROCESSES];
    static uint32_t s_time;
    static uint32_t s_processCount;

    static void timer(CPUStatus* status);

    static void purgeProcesses();
};