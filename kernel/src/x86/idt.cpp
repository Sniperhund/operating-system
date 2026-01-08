#include "idt.h"
#include "drivers/text.h"
#include "gdt.h"
#include <stdio.h>

IDT::Descriptor IDT::s_idt[IDT_SIZE];
IDT::IDTR IDT::s_idtr;

int IDT::init() {
    s_idtr.base = (uintptr_t)&s_idt[0];
    s_idtr.limit = (uint16_t)sizeof(Descriptor) * IDT_SIZE - 1;

    extern void* isr_stub_table[];

    for (uint8_t i = 0; i < 32; i++) {
        setDescriptor(i, isr_stub_table[i], 0x8E);
    }

    extern void* irq_stub_table[];

    for (uint8_t i = 0; i < 16; i++) {
        setDescriptor(i + 32, irq_stub_table[i], 0x8E);
    }

    asm volatile("lidt %0" : : "m"(s_idtr));
    asm volatile("sti");

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

void IDT::exceptionHandler(CPUStatus* status) {
    printf("Exception: 0x%X\nError code: %x\n", status->intNo, status->errCode);

    if (status->intNo == 0xE) {
        unsigned long val;
        asm volatile ( "mov %%cr2, %0" : "=r"(val) );
        printf("CR2: 0x%x", val);
    }

    while (true) {
        asm volatile("cli; hlt");
    }
}

extern "C" void exceptionHandlerC(CPUStatus* status) {
    IDT::exceptionHandler(status);
}