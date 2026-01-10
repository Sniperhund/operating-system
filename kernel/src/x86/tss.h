#pragma once

#include <stdint.h>

class TSS {
public:
    struct Entry {
        uint32_t prevTSS;
        uint32_t esp0;
        uint32_t ss0;
        // ESP/SS 1 and 2 would be used when switching to rings 1 or 2
        uint32_t esp1;
        uint32_t ss1;
        uint32_t esp2;
        uint32_t ss2;
        uint32_t cr3;
        uint32_t eip;
        uint32_t eax;
        uint32_t ecx;
        uint32_t edx;
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        uint32_t es;
        uint32_t cs;
        uint32_t ss;
        uint32_t ds;
        uint32_t fs;
        uint32_t gs;
        uint32_t ldt;
        uint16_t trap;
        uint16_t iomapBase;
    } __attribute__((packed));

    /**
     * Writes the TSS to the GDT
     */
    static void writeTSS(uint32_t index);

private:
    static Entry s_tss;
};