#include "idt.h"
#include "interrupt.h"
#include "gdt.h"
#include <string.h>
#include <stdio.h>

IDT::Descriptor IDT::s_idt[IDT_SIZE];
IDT::IDTR IDT::s_idtr;
IDT::isrHandlerFunc IDT::s_isr[32];

int IDT::init() {
    s_idtr.base = (uintptr_t)&s_idt[0];
    s_idtr.limit = (uint16_t)sizeof(Descriptor) * IDT_SIZE - 1;

    memset(s_isr, 0, sizeof(s_isr));

    extern void* isr_stub_table[];

    for (uint8_t i = 0; i < 32; i++) {
        setDescriptor(i, isr_stub_table[i], 0x8E);
    }

    extern void* irq_stub_table[];

    for (uint8_t i = 0; i < 16; i++) {
        setDescriptor(i + 32, irq_stub_table[i], 0x8E);
    }

    extern void* syscall_stub;

    setDescriptor(0x80, &syscall_stub, 0xEE);

    asm volatile("lidt %0" : : "m"(s_idtr));
    sti();

    return 0;
}

void IDT::setDescriptor(uint8_t index, void* isr, uint8_t flags) {
    Descriptor* desc = &s_idt[index];

    desc->isrLow        = (uint32_t)isr & 0xFFFF;
    desc->kernelCS      = KERNEL_CODE_SELECTOR;
    desc->attributes    = flags;
    desc->isrHigh       = (uint32_t)isr >> 16;
    desc->reserved      = 0;
}

void IDT::exceptionHandler(CPUStatus* s) {
    int result = -1;

    if (s_isr[s->intNo]) {
        result = s_isr[s->intNo](s);
    } else {
        printf("Exception: 0x%X\nError code: %x\n", s->intNo, s->errCode);
        result = PRINT_HALT;
    }

    switch(result) {
        case RECOVER:
            return;
        case PRINT_RECOVER:
            goto print;
        case HALT:
            goto halt;
        case PRINT_HALT:
            goto print;
    }

print:
    printf("DS: 0x%x, EDI: 0x%x, ESI: 0x%x, EBP: 0x%x, ESP: 0x%x, EBX: 0x%x, EDX: 0x%x, ECX: 0x%x, EAX: 0x%x, EIP: 0x%x\n",
        s->ds, s->edi, s->esi, s->ebp, s->esp, s->ebx, s->edx, s->ecx, s->eax, s->eip);

    if (result == PRINT_RECOVER)
        return;

halt:
    while (true) {
        asm volatile("cli; hlt");
    }
}

extern "C" void exceptionHandlerC(CPUStatus* status) {
    IDT::exceptionHandler(status);
}

void IDT::registerExceptionHandler(uint8_t isr, isrHandlerFunc func) {
    if (func) s_isr[isr] = func;
}

void cli() {
    asm volatile("cli");
}

void sti() {
    asm volatile("sti");
}