#pragma once

#include <stdint.h>

#define ACCESS_PRESENT      (1 << 7)
#define ACCESS_DPL(dpl)     ((dpl & 0x3) << 5)
#define ACCESS_SYSTEM       0        // S bit = 0, e.g. TSS, LDT, gates
#define ACCESS_DESCRIPTOR   (1 << 4) // S bit = 1, e.g. code/data
#define ACCESS_CODE         (1 << 3)
#define ACCESS_DATA         0
#define ACCESS_RW           (1 << 1)
#define ACCESS_EXEC_READ    (1 << 1) // Is it readable when code segment

#define ACCESS_TSS_32       ((1 << 3) | (1 << 0))
#define ACCESS_TSS_BUSY     ((1 << 3) | (1 << 1) | (1 << 0))

#define FLAG_GRANULARITY    (1 << 7)
#define FLAG_SIZE_32        (1 << 6)
#define FLAG_TSS            0

#define GDT_SIZE 8

#define KERNEL_CODE_SELECTOR (1 << 3)
#define KERNEL_DATA_SELECTOR (2 << 3)

class GDT {
public:
    struct Descriptor {
        uint16_t limitLow;
        uint16_t baseLow;
        uint8_t baseMid;
        uint8_t access;
        uint8_t flagsLimit; // flags (high) + limitHigh
        uint8_t baseHigh;
    } __attribute__((packed));

    /**
     * @param debfaultDescriptors Calls defaultDescriptors() itself
     */
    static int init(bool defaultDescriptors = false);
    static void defaultDescriptors();

    static void setDescriptor(uint32_t index, const Descriptor& desc);

private:
    struct GDTR {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    static Descriptor s_gdt[GDT_SIZE];
    static GDTR s_gdtr;

    static uint8_t s_tssIndex;
};

static_assert(sizeof(GDT::Descriptor) == 8, "GDT Descriptor MUST be 8 bytes");