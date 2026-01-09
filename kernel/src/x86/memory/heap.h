#pragma once

#include <stdint.h>
#include <stddef.h>

class Heap {
public:
    static int init(void* start, size_t size);

    static void* alloc(size_t size);
    static void* allocAligned(size_t size, size_t alignment);
    static void free(void* ptr);
    static void* realloc(void* ptr, size_t size);
    
private:
    struct Header {
        size_t size;
        bool used;
        Header* next;
    };

    static Header* s_start;
    static uintptr_t s_end;

    static constexpr size_t MIN_BLOCK_SIZE = sizeof(Header) + 8;
};