#include "gdt.h"
#include "panic.h"

GDT::Descriptor GDT::s_gdt[GDT_SIZE];
GDT::GDTR GDT::s_gdtr;

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

    return 0;
}

void GDT::defaultDescriptors() {
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
        ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_DESCRIPTOR | ACCESS_RW | ACCESS_DATA,
        (FLAG_GRANULARITY | FLAG_SIZE_32) | 0x0F,
        0
    });
}

void GDT::setDescriptor(uint32_t index, const Descriptor& desc) {
    if (index > GDT_SIZE) panic("GDT", "Index over GDT_SIZE");
    
    s_gdt[index] = desc;
}