#include "paging.h"
#include "panic.h"
#include "x86/idt.h"
#include "x86/memory/heap.h"
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

    s_kernelDir = (PD*)PageHeap::allocPage(2);
    uint32_t frame = PMM::physToFrame((void*)((uintptr_t)s_kernelDir - LOAD_MEMORY_ADDRESS));
    PMM::mark(frame);
    PMM::mark(frame + 1);
    memset(s_kernelDir, 0, sizeof(PD));

    mapregion(nullptr, 0, (void*)0x100000, 0);
    mapregion(nullptr, (void*)(LOAD_MEMORY_ADDRESS), (void*)(LOAD_MEMORY_ADDRESS + 0x800000), 0);

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

int Paging::mapregion(void* vDir, void* virtStart, void* virtEnd, void* physStart) {
    if (!vDir) vDir = s_kernelDir;
    PD* dir = (PD*)vDir;
    
    uint32_t vStart = PAGE_ALIGNDOWN(virtStart);
    // FIX: This is technically incorrect, but it fixes a page fault (until I have a kernel heap)
    uint32_t vEnd = PAGE_ALIGNUP(virtEnd);
    uint32_t pStart = PAGE_ALIGNDOWN(physStart);

    while (vStart < vEnd) {
        mappage(dir, (void*)vStart, PMM::physToFrame((void*)pStart));
        vStart += PAGE_SIZE;
        pStart += PAGE_SIZE;
    }

    return 0;
}

int Paging::mappage(void *vDir, void *virt, size_t frame) {
    if (!vDir) vDir = s_kernelDir;
    PD* dir = (PD*)vDir;

    uint32_t pdIdx = PD_INDEX(virt), ptIdx = PT_INDEX(virt);
    
    PT* table = dir->refTables[pdIdx];
    if (!table) {
        table = (PT*)(PD*)PageHeap::allocPage();
        uint32_t ptFrame = PMM::physToFrame((void*)((uintptr_t)table - LOAD_MEMORY_ADDRESS));
        PMM::mark(ptFrame);

        memset((void*)((uint32_t)table), 0, 4096);

        dir->tables[pdIdx].frame = ptFrame;
        dir->tables[pdIdx].present = 1;
        dir->tables[pdIdx].rw = 1;
        dir->tables[pdIdx].user = 1;
        dir->tables[pdIdx].page_size = 0;

        dir->refTables[pdIdx] = table;
    }

    if (!table->pages[ptIdx].present) {
        if (frame) table->pages[ptIdx].frame = frame;
        else {
            void* ptr = (PD*)PageHeap::allocPage();
            uint32_t ptFrame = PMM::physToFrame((void*)((uintptr_t)ptr - LOAD_MEMORY_ADDRESS));
            table->pages[ptIdx].frame = ptFrame;
        }

        PMM::mark(table->pages[ptIdx].frame);

        table->pages[ptIdx].present = 1;
        table->pages[ptIdx].rw = 1;
        table->pages[ptIdx].user = 1;
    }

    asm volatile("invlpg (%0)" :: "r"(virt) : "memory");

    return 0;
}

void Paging::switchPD(PD *dir, bool isPhysAddr) {
    if (!dir) PANIC("Paging", "Page directory is empty");

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
    return s_currentDir;
}

void* mmap(void* addr, size_t length, uint8_t prot) {
    return mmap(Paging::currentPD(), addr, length, prot);
}

void* mmap(void* dir, void* addr, size_t length, uint8_t prot) {
    if (!addr) return nullptr;

    length = PAGE_ALIGNUP(length);

    for (size_t i = 0; i < length; i += PAGE_SIZE) {
        size_t frame = PMM::findFirstFreeFrame();
        PMM::mark(frame);
        Paging::mappage(dir, (void*)((uintptr_t)addr + i), frame);
    }

    return addr;
}