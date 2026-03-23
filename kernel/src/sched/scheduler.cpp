#include "scheduler.h"
#include "exec/process.h"
#include "panic.h"
#include "x86/idt.h"
#include "x86/interrupt.h"
#include "x86/irq.h"
#include "x86/memory/paging.h"
#include "x86/pic.h"
#include "x86/tss.h"
#include <stdio.h>
#include <string.h>

Proc* Scheduler::s_processes[MAX_PROCESSES];
uint32_t Scheduler::s_time = 0;
uint32_t Scheduler::s_processCount = 0;

Proc* current = nullptr;

int Scheduler::init() {
    memset(s_processes, 0, sizeof(s_processes));
    return 0;
}

void Scheduler::addProcess(Proc *proc) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!s_processes[i]) {
            s_processes[i] = proc;
            s_processCount++;
            return;
        }
    }

    PANIC("Scheduler", "Too many processes");
}

void Scheduler::run() {
    // Now we wait for a timer IRQ

    IRQ::registerIRQ(0, timer);

    while (true) {
        asm volatile("hlt");
    }
}

uint32_t currentProc = -1;

// This is called from the timer IRQ
void Scheduler::switchTask(CPUStatus *cpu) {
    // NOTE: This shouldn't be called before Scheduler::run

    cli();

    if (current) {
        CPUContext* ctx = &current->ctx;
        ctx->eax = cpu->eax;
        ctx->ebx = cpu->ebx;
        ctx->ecx = cpu->ecx;
        ctx->edx = cpu->edx;
        ctx->esi = cpu->esi;
        ctx->edi = cpu->edi;
        ctx->ebp = cpu->ebp;
        ctx->esp = cpu->esp;
        ctx->eip = cpu->eip;

        ctx->eflags = cpu->eflags;
        if ((cpu->cs & 0x3) == 3) { // If true CPL == 3
            ctx->useresp = cpu->useresp;
            ctx->ss = cpu->ss;
        }

        if (current->state == RUNNING)
            current->state = READY;
    }

    purgeProcesses();

    // Change to actually switch
    Proc* next = nullptr;
    uint32_t attempts = 0;

    // TODO: Make it handle fragmented processes in array, if they get purged
    while (attempts < s_processCount) {
        currentProc = (currentProc + 1) % s_processCount;
        next = s_processes[currentProc];

        if (next && next->state == READY)
            break;

        attempts++;
    }

    if (next && next->state == READY) {
        next->state = RUNNING;
        switchTo(next);
    }

    sti();

    while (1) { asm volatile("hlt"); }
}

Proc* Scheduler::getByPid(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        Proc* proc = s_processes[i];

        if (proc && proc->pid == pid) return proc;
    }

    return nullptr;
}

void Scheduler::timer(CPUStatus* status) {
    s_time++;

    if (s_time % 10 == 0)
        switchTask(status);
}

void Scheduler::purgeProcesses() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        Proc* proc = s_processes[i];

        if (proc != current && (proc->state == KILLED || proc->state == EXITED)) {
            Proc::freeProcess(proc);
            s_processes[i] = nullptr;
            s_processCount--;
        }
    }
}

extern "C" void __attribute__((naked)) usermode(CPUContext* ctx);

void Scheduler::switchTo(Proc* next) {
    if (!next) return;

    if (next->pd)
        Paging::switchPD(next->pd, false);

    TSS::setKernelESP((uintptr_t)next->ctx.esp);
    current = next;

    PIC::sendEOI(0);

    usermode(&next->ctx);
}