#pragma once

#include <stdint.h>
#include <stddef.h>

class Heap {
public:
    static int init(void* start, size_t size, bool debug);

    static void* alloc(size_t size);
    static void* allocAligned(size_t size, size_t alignment);
    static void free(void* ptr);
    static void* realloc(void* ptr, size_t size);
    
private:
    static void redrawDebug();
    static void drawDebugBoxBorder();

    struct Header {
        size_t size;
        bool used;
        Header* next;
        uint32_t magic;
    };

    struct DebugInfo {
        size_t used;
        size_t free;
        size_t usedOnHeaders;
    };

    static Header* s_start;
    static uintptr_t s_end;

    static bool s_debug;
    static DebugInfo s_debugInfo;

    static constexpr size_t MIN_BLOCK_SIZE = sizeof(Header) + 8;
    static constexpr uint32_t MAGIC = 0xDEADBEEF;
};