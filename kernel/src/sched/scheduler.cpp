#include "scheduler.h"
#include "exec/process.h"
#include "panic.h"
#include "x86/idt.h"
#include "x86/irq.h"
#include "x86/memory/paging.h"
#include "x86/pic.h"
#include "x86/tss.h"
#include <string.h>

Proc* Scheduler::s_processes[MAX_PROCESSES];
uint32_t Scheduler::s_time = 0;
uint32_t Scheduler::s_processCount = 0;

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

#include <stdio.h>

uint32_t currentProc = -1;

// This is called from the timer IRQ
void Scheduler::switchTask(CPUStatus *cpu) {
    // This shouldn't be called before Scheduler::run

    asm volatile("cli");

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
        ctx->useresp = cpu->useresp;
        ctx->ss = cpu->ss;
    }

    // Change to actually switch
    currentProc++;
    printf("Switching to %d process\n", currentProc % s_processCount);
    switchTo(current, s_processes[currentProc % s_processCount]);
    

    asm volatile("sti");
}

void Scheduler::timer(CPUStatus* status) {
    s_time++;

    if (s_time % 10 == 0)
        switchTask(status);
}

extern "C" void __attribute__((naked)) usermode(CPUContext* ctx);

void Scheduler::switchTo(Proc* prev, Proc* next) {
    if (!next) return;

    if (next->pd)
        Paging::switchPD(next->pd, false);

    TSS::setKernelESP((uintptr_t)next->ctx.esp);
    current = next;

    PIC::sendEOI(0);

    usermode(&next->ctx);
}