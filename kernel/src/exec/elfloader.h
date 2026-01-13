#pragma once

#include "stdint.h"

class ELFLoader {
public:
    static int loadBinary();
    static uint8_t loadExecutable(void* file);

    enum Error {
        NONE = 0,
        VERIFATION,
        ARCH,
        ENDIAN,
        INST_SET,
        HEADER_VER,
        TYPE,
    };

private:
    struct Header {
        unsigned char ident[4];
        uint8_t arch;           // 1 for x86, 2 for x86_64
        uint8_t endian;         // 1 for little endian, 2 for big endian
        uint8_t headerVer;
        uint8_t abi;            // Usually 0 for System V
        char padding[8];
        uint16_t type;          // 1 for relocatable, 2 for executable, 3 for shared, 4 for core
        uint16_t instructionSet;
        uint32_t elfVersion;
        uint32_t entry;         // Program entry offset
        uint32_t phoff;         // Program header offset
        uint32_t shoff;         // Section header offset
        uint32_t flags;         // Can probably be ignored for x86
        uint16_t size;          // Header size
        uint16_t phSize;        // Size of each program header
        uint16_t phNum;         // Number of program headers
        uint16_t shSize;        // Size of each section header
        uint16_t shNum;         // Number of program headers
        uint16_t shStrNdx;      // Section index to the section header string table
    };

    struct ProgramHeader {
        uint32_t type;
        uint32_t offset;
        uint32_t vaddr;         // Virtual load address
        uint32_t paddr;         // Physical load address
        uint32_t fileSize;      // Size of the segment in the file
        uint32_t memSize;       // Size of data in memory, any excess over disk size is zero'd
        uint32_t flags;
        uint32_t align;         // Alignment
    };

    enum Type {
        ET_NONE     = 0, // Unknown
        ET_REL      = 1, // Relocatable file
        ET_EXEC     = 2, // Executable file
    };

    static bool verifyBinary(Header* header);
    static uint8_t checkSupported(Header* header);
};