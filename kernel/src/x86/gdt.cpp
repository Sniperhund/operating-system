#include "gdt.h"
#include "debug.h"
#include "panic.h"
#include "x86/tss.h"

GDT::Descriptor GDT::s_gdt[GDT_SIZE];
GDT::GDTR GDT::s_gdtr;

uint8_t GDT::s_tssIndex = 0;

int GDT::init(bool defaultDesc) {
    if (defaultDesc) defaultDescriptors();

    s_gdtr.base = (uintptr_t)&s_gdt;
    s_gdtr.limit = sizeof(s_gdt) - 1;

    asm volatile ("lgdt %0" : : "m"(s_gdtr));

    asm volatile (
        "ljmp $0x08, $flush_cs\n"
        "flush_cs:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%es \n"
        "mov %%ax, %%fs \n"
        "mov %%ax, %%gs \n"
        "mov %%ax, %%ss \n"
        : : : "ax", "memory"
    );

    if (s_tssIndex != 0) {
        uint16_t tssSelector = (5 << 3) | 0; // Ring 0
        asm volatile("ltr %0" : : "r"(tssSelector));
    }

    return 0;
}

void GDT::defaultDescriptors() {
    BOCHS_BREAK;

    // Null descriptor
    setDescriptor(0, { 0, 0, 0, 0, 0, 0 });

    // Kernel code segment
    setDescriptor(1, {
        0xFFFF,
        0,
        0,
        ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_CODE | ACCESS_RW | ACCESS_DESCRIPTOR,
        (FLAG_GRANULARITY | FLAG_SIZE_32) | 0x0F,
        0
    });

    // Kernel data segment
    setDescriptor(2, {
        0xFFFF,
        0,
        0,
        ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_DATA | ACCESS_RW | ACCESS_DESCRIPTOR,
        (FLAG_GRANULARITY | FLAG_SIZE_32) | 0x0F,
        0
    });

    // User code segment
    setDescriptor(3, {
        0xFFFF,
        0,
        0,
        ACCESS_PRESENT | ACCESS_DPL(3) | ACCESS_CODE | ACCESS_RW | ACCESS_DESCRIPTOR,
        (FLAG_GRANULARITY | FLAG_SIZE_32) | 0x0F,
        0
    });

    // User data segment
    setDescriptor(4, {
        0xFFFF,
        0,
        0,
        ACCESS_PRESENT | ACCESS_DPL(3) | ACCESS_DATA | ACCESS_RW | ACCESS_DESCRIPTOR,
        (FLAG_GRANULARITY | FLAG_SIZE_32) | 0x0F,
        0
    });

    s_tssIndex = 5;
    TSS::writeTSS(s_tssIndex);
}

void GDT::setDescriptor(uint32_t index, const Descriptor& desc) {
    if (index > GDT_SIZE) PANIC("GDT", "Index over GDT_SIZE");
    
    s_gdt[index] = desc;
}