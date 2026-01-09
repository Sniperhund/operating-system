#pragma once

#include "x86/idt.h"
#include <stdint.h>
#include <stddef.h>

#define PD_INDEX(virt) (((uint32_t)virt) >> 22)
#define PT_INDEX(virt) ((((uint32_t)virt) >> 12) & 0x3ff)
#define PAGE_OFFSET(virt) (((uint32_t)virt) & 0xfff)

#define PAGE_SIZE 4096

class Paging {
private:
    static bool enabled;

    struct PDEntry {
        uint32_t present    : 1;
        uint32_t rw         : 1;
        uint32_t user       : 1;
        uint32_t w_through  : 1;
        uint32_t cache      : 1;
        uint32_t access     : 1;
        uint32_t reserved   : 1;
        uint32_t page_size  : 1;
        uint32_t global     : 1;
        uint32_t available  : 3;
        uint32_t frame      : 20;
    };

    struct PTEntry {
        uint32_t present    : 1;
        uint32_t rw         : 1;
        uint32_t user       : 1;
        uint32_t reserved   : 2;
        uint32_t accessed   : 1;
        uint32_t dirty      : 1;
        uint32_t reserved2  : 2;
        uint32_t available  : 3;
        uint32_t frame      : 20;
    };

    struct PT
    {
        PTEntry pages[1024];
    };

    struct PD
    {
        // The actual page directory entries(note that the frame number it stores is physical address)
        PDEntry tables[1024];
        // We need a table that contains virtual address, so that we can actually get to the tables
        PT* refTables[1024];
    };

    static PD* s_kernelDir;
    static PD* s_currentDir;

    static int pageFaultHandler(CPUStatus* status);

public:
    static int init();

    static void* virtToPhys(PD* dir, void* virt);

    static int mapregion(PD* dir, void* virtStart, void* virtEnd, void* physStart);
    static int mappage(PD* dir, void* virt, size_t frame);
    
    static void switchPD(PD *dir, bool isPhysAddr);

    static PD* currentPD();
};