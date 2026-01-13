#include "elfloader.h"
#include "x86/memory/paging.h"
#include "x86/memory/pmm.h"
#include <string.h>

#define ELF_ARCH_X86        (1)
#define ELF_LITTLE_ENDIAN   (1)
#define ELF_MACH_386        (3) // x86 Instruction set type

#define PT_LOAD             (1)

uint8_t ELFLoader::loadExecutable(void* file) {
    Header* hdr = (Header*)file;

    if (hdr->type != ET_EXEC) return TYPE;

    ProgramHeader* ph = (ProgramHeader*)((uint8_t*)file + hdr->phoff);

    for (int i = 0; i < hdr->phNum; i++) {
        if (ph[i].type != PT_LOAD)
            continue;

        uint32_t vaddr = ph[i].vaddr & 0xFFFFF000;
        uint32_t diff = ph[i].vaddr - vaddr;
        uint32_t size = PAGE_ALIGNUP(ph[i].memSize + diff);

        for (uint32_t j = 0; j < size; j += PAGE_SIZE) {
            mmap((void*)(vaddr + j), PAGE_SIZE, 0);
        }

        void* src = (uint8_t*)file + ph[i].offset;
        void* dst = (void*)ph[i].vaddr;

        memcpy(dst, src, ph[i].fileSize);

        if (ph[i].memSize > ph[i].fileSize) {
            memset((uint8_t*)dst + ph[i].fileSize, 0, ph[i].memSize - ph[i].fileSize);
        }
    }

    asm volatile("jmp *%0" :: "r"(hdr->entry));

    return NONE;
}

uint8_t ELFLoader::checkSupported(Header *hdr) {
    if (!verifyBinary(hdr)) return VERIFATION;
    if (hdr->arch != ELF_ARCH_X86) return ARCH;
    if (hdr->endian != ELF_LITTLE_ENDIAN) return ENDIAN;
    if (hdr->instructionSet != ELF_MACH_386) return INST_SET;
    if (hdr->headerVer != 1) return HEADER_VER; // ELF current version
    if (hdr->type != ET_REL && hdr->type != ET_EXEC) return TYPE;

    return NONE;
}

bool ELFLoader::verifyBinary(Header* hdr) {
    if (!hdr) return false;

    if (hdr->ident[0] != 0x7F || hdr->ident[1] != 'E' || hdr->ident[2] != 'L' || hdr->ident[3] != 'F')
        return false;

    return true;
}