#include "tss.h"
#include "x86/gdt.h"
#include <string.h>

TSS::Entry TSS::s_tss;

void TSS::writeTSS(uint32_t index) {
    uint32_t base = (uint32_t)&s_tss;
    uint32_t limit = sizeof(Entry) - 1;

    GDT::setDescriptor(index, {
        (uint16_t)limit,
        (uint16_t)base,
        (uint8_t)((base >> 16) & 0xFF),
        //ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_SYSTEM | ACCESS_TSS_32,
        0x89,
        (uint8_t)((limit & (0xF << 16)) >> 16),
        (uint8_t)((base >> 24) & 0xFF)
    });

    memset(&s_tss, 0, sizeof(Entry));

    s_tss.iomapBase = sizeof(Entry);
    s_tss.ss0 = KERNEL_DATA_SELECTOR;
    asm volatile("mov %%esp, %0" : "=r"(s_tss.esp0) : :);
}

void TSS::setKernelESP(uint32_t esp) {
    s_tss.esp0 = esp;
}