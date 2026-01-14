#include "paging.h"
#include "error.h"
#include "panic.h"
#include "x86/idt.h"
#include "x86/memory/pageHeap.h"
#include "x86/memory/pmm.h"
#include <string.h>
#include <stdio.h>

#define LOAD_MEMORY_ADDRESS 0xC0000000

bool Paging::enabled = false;
Paging::PD* Paging::s_kernelDir = nullptr;
Paging::PD* Paging::s_currentDir = nullptr;

extern char kernel_start[];
extern char kernel_end[];

static_assert(LOAD_MEMORY_ADDRESS % PAGE_SIZE == 0, "LOAD_MEMORY_ADDRESS macro is not page aligned");

int Paging::init() {
    IDT::registerExceptionHandler(0xE, Paging::pageFaultHandler);

    // Mark reserved 0-1MB 
    for (uint32_t addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
        PMM::mark(PMM::physToFrame((void*)addr));
    }

    // Mark kernel itself
    uint32_t phys_start = (uint32_t)kernel_start - LOAD_MEMORY_ADDRESS;
    uint32_t phys_end   = (uint32_t)kernel_end   - LOAD_MEMORY_ADDRESS;

    for (uint32_t addr = phys_start; addr < phys_end; addr += PAGE_SIZE) {
        PMM::mark(PMM::physToFrame((void*)addr));
    }

    // PD have 4KiB of x86 PD and 4KiB of ref tables (Just pointer to the first 4KiB tables)
    s_kernelDir = (PD*)PageHeap::allocPage(2);

    if (s_kernelDir == nullptr) return E_NOMEM;

    uint32_t frame = PMM::physToFrame((void*)((uintptr_t)s_kernelDir - LOAD_MEMORY_ADDRESS));
    PMM::mark(frame);
    PMM::mark(frame + 1);
    memset(s_kernelDir, 0, sizeof(PD));

    mapregion(nullptr, 0, (void*)0x100000, 0, PROT_WRITE | PROT_KERNEL);
    mapregion(nullptr, (void*)(LOAD_MEMORY_ADDRESS), (void*)(LOAD_MEMORY_ADDRESS + 0x800000), 0, PROT_WRITE | PROT_KERNEL);

    switchPD(s_kernelDir, false);

    uint32_t cr4;

    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 = cr4 & 0xffffffef;
    asm volatile("mov %0, %%cr4" :: "r"(cr4));

    enabled = true;

    return 0;
}

void* Paging::virtToPhys(PD* dir, void* virt) {
    if (!dir) dir = s_kernelDir;

    // This is only for the time between TEMP_PD and the real paging gets enabled
    if (!enabled) return (void*)((uint32_t)virt - LOAD_MEMORY_ADDRESS);

    uint32_t pdIdx = PD_INDEX(virt), ptIdx = PT_INDEX(virt), offset = PAGE_OFFSET(virt);
    
    if (!dir->refTables[pdIdx]) return nullptr;
    PT* pt = dir->refTables[pdIdx];

    if (!pt->pages[ptIdx].present) return nullptr;
    uint32_t frame = pt->pages[ptIdx].frame;
    
    return (void*)((frame << 12) + offset);
}

int Paging::mapregion(void* vDir, void* virtStart, void* virtEnd, void* physStart, uint8_t prot) {
    if (!vDir) vDir = s_kernelDir;
    PD* dir = (PD*)vDir;
    
    uint32_t vStart = PAGE_ALIGNDOWN(virtStart);
    uint32_t vEnd = PAGE_ALIGNUP(virtEnd);
    uint32_t pStart = PAGE_ALIGNDOWN(physStart);

    while (vStart < vEnd) {
        int result = mappage(dir, (void*)vStart, PMM::physToFrame((void*)pStart), prot);

        if (result != 0) return result;

        vStart += PAGE_SIZE;
        pStart += PAGE_SIZE;
    }

    return 0;
}

int Paging::mappage(void *vDir, void *virt, size_t frame, uint8_t prot) {
    if (!vDir) vDir = s_kernelDir;
    PD* dir = (PD*)vDir;

    uint32_t pdIdx = PD_INDEX(virt), ptIdx = PT_INDEX(virt);
    
    PT* table = dir->refTables[pdIdx];
    if (!table) {
        table = (PT*)(PD*)PageHeap::allocPage();

        if (table == nullptr) return E_NOMEM; 

        uint32_t ptFrame = PMM::physToFrame((void*)((uintptr_t)table - LOAD_MEMORY_ADDRESS));
        PMM::mark(ptFrame);

        memset((void*)((uint32_t)table), 0, 4096);

        dir->tables[pdIdx].frame = ptFrame;
        dir->tables[pdIdx].present = (prot != PROT_NONE) ? 1 : 0;
        dir->tables[pdIdx].rw = (prot & PROT_WRITE) ? 1 : 0;
        dir->tables[pdIdx].user = (prot & PROT_KERNEL) ? 0 : 1;
        dir->tables[pdIdx].page_size = 0;

        dir->refTables[pdIdx] = table;
    }

    if (!table->pages[ptIdx].present) {
        if (frame) table->pages[ptIdx].frame = frame;
        else {
            void* ptr = (PD*)PageHeap::allocPage();

            if (ptr == nullptr) return E_NOMEM;

            uint32_t ptFrame = PMM::physToFrame((void*)((uintptr_t)ptr - LOAD_MEMORY_ADDRESS));
            table->pages[ptIdx].frame = ptFrame;
        }

        PMM::mark(table->pages[ptIdx].frame);

        table->pages[ptIdx].present = (prot != PROT_NONE) ? 1 : 0;
        table->pages[ptIdx].rw = (prot & PROT_WRITE) ? 1 : 0;
        table->pages[ptIdx].user = (prot & PROT_KERNEL) ? 0 : 1;
    } else {
        return E_INVAL;
    }

    asm volatile("invlpg (%0)" :: "r"(virt) : "memory");

    return 0;
}

int Paging::unmappage(void *vDir, void *virt) {
    if ((uintptr_t)virt >= LOAD_MEMORY_ADDRESS) {
        printf("Process attempted to ummap kernel memory at 0x%x! Killing process.\n", virt);
        return E_KILL;
    }

    if (!vDir) vDir = s_kernelDir;
    PD* dir = (PD*)vDir;

    uint32_t pdIdx = PD_INDEX(virt), ptIdx = PT_INDEX(virt);
    PT* table = dir->refTables[pdIdx];

    if (!table || !table->pages[ptIdx].present) return E_INVAL;
        
    PMM::unmark(table->pages[ptIdx].frame);
    table->pages[ptIdx].present = 0;

    asm volatile("invlpg (%0)" :: "r"(virt) : "memory");

    return 0;
}

Paging::PD* Paging::createPD() {
    PD* pd = (PD*)PageHeap::allocPage(2);

    if (!pd) {
        s_error = E_NOMEM;
        return nullptr;
    }

    uint32_t frame = PMM::physToFrame((void*)((uintptr_t)pd - LOAD_MEMORY_ADDRESS));
    PMM::mark(frame);
    PMM::mark(frame + 1);
    memset(pd, 0, sizeof(PD));

    mapregion(pd, 0, (void*)0x100000, 0, PROT_WRITE);
    mapregion(pd, (void*)(LOAD_MEMORY_ADDRESS), (void*)(LOAD_MEMORY_ADDRESS + 0x800000), 0, PROT_WRITE | PROT_KERNEL);

    return pd;
}

void Paging::switchPD(void *vDir, bool isPhysAddr) {
    if (!vDir) PANIC("Paging", "Page directory is empty");
    PD* dir = (PD*)vDir;

    uint32_t phys = 0;
    if (isPhysAddr) phys = (uint32_t)dir;
    else phys = (uint32_t)dir - LOAD_MEMORY_ADDRESS;

    s_currentDir = dir;
    
    asm volatile("mov %0, %%cr3" : : "r"(phys));
}

#define TEST_BIT(err, shift) ((err >> shift) & 0b1) == 1

int Paging::pageFaultHandler(CPUStatus* status) {
    printf("Page fault (0x%x)\n", status->intNo);

    bool present = TEST_BIT(status->errCode, 0);
    bool write = TEST_BIT(status->errCode, 1);
    bool user = TEST_BIT(status->errCode, 2);
    bool instructionFetch = TEST_BIT(status->errCode, 4);
    bool protectionKey = TEST_BIT(status->errCode, 5);
    bool shadowStack = TEST_BIT(status->errCode, 6);
    
    printf("P: %d, W: %d, U: %d, I: %d, PK: %d, SS: %d\n", present, write, user, instructionFetch, protectionKey, shadowStack);

    unsigned long val;
    asm volatile ( "mov %%cr2, %0" : "=r"(val) );
    printf("CR2: 0x%x\n", val);

    return IDT::PRINT_HALT;
}

Paging::PD* Paging::currentPD() {
    // FIX: This may return a physical addr, not mapped
    return s_currentDir;
}

void* mmap(void* addr, size_t length, uint8_t prot) {
    return mmap(Paging::currentPD(), addr, length, prot);
}

void* mmap(void* dir, void* addr, size_t length, uint8_t prot) {
    if (!addr) {
        s_error = E_INVAL;
        return nullptr;
    }

    length = PAGE_ALIGNUP(length);

    for (size_t i = 0; i < length; i += PAGE_SIZE) {
        size_t frame = PMM::findFirstFreeFrame();
        PMM::mark(frame);
        int result = Paging::mappage(dir, (void*)((uintptr_t)addr + i), frame, prot);
        
        if (result) {
            // Clean up after ourselves since it failed.
            if (i != 0) munmap(dir, addr, i);
            s_error = result;
            return (void*)-1;
        }
    }

    return addr;
}

int munmap(void* addr, size_t length) {
    return munmap(Paging::currentPD(), addr, length);
}

int munmap(void *dir, void *addr, size_t length) {
    if (!addr) return E_INVAL;

    length = PAGE_ALIGNUP(length);

    for (size_t i = 0; i < length; i += PAGE_SIZE) {
        int result = Paging::unmappage(dir, (void*)((uintptr_t)addr + i));

        if (result) {
            return result;            
        }
    }

    return 0;
}