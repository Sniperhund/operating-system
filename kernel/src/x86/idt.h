#pragma once

#include <stdint.h>

#define IDT_SIZE 256

struct CPUStatus {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t intNo, errCode;
    uint32_t eip, cs, eflags, useresp, ss;
};

extern "C" void exceptionHandlerC(CPUStatus* status);

class IDT {
public:
    struct Descriptor {
        uint16_t isrLow;
        uint16_t kernelCS;
        uint8_t reserved;
        uint8_t attributes;
        uint16_t isrHigh;
    } __attribute__((packed));

    static void init();

private:
    struct IDTR {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    __attribute__((aligned(0x10))) static Descriptor s_idt[IDT_SIZE];
    static IDTR s_idtr;

    static void setDescriptor(uint8_t index, void* isr, uint8_t flags);

    friend void ::exceptionHandlerC(CPUStatus* status);

    static void exceptionHandler(CPUStatus* status);
};

static_assert(sizeof(IDT::Descriptor) == 8, "IDT Descriptor MUST be 8 bytes");