#include "heap.h"
#include "panic.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

Heap::Header* Heap::s_start = nullptr;
uintptr_t Heap::s_end = 0;

/**
 * Only int 0x80 (syscalls) use the heap for now, so it's unnecessary to use `cli`
 * Though this may change when 0xE (page faults) are handled correctly since that uses the PageHeap for allocation
 * If the memory accessed from 0x80 is required to allocate a whole new page (which I think is unlikely, but you never know)
 *
 * Spinlocks should be implemented though.
 */
int Heap::init(void *start, size_t size) {
    s_start = (Heap::Header*)start;

    s_start->size = size - sizeof(Header);
    s_start->next = nullptr;
    s_start->used = false;
    s_start->magic = MAGIC;

    s_end = (uintptr_t)start + size;

    return 0;
}

#define GET_HEADER(ptr) ((Heap::Header*)((uintptr_t)(ptr) - sizeof(Heap::Header)))
#define GET_BLOCK(ptr) ((void*)((uintptr_t)(ptr) + sizeof(Heap::Header)))
#define GET_FULL_BLOCK_SIZE(size) (size + sizeof(Heap::Header))

void* Heap::alloc(size_t payloadSize) {
    Header* current = s_start;
    payloadSize = (payloadSize + 7) & ~7;

    while (current && (uintptr_t)current < s_end) {
        if (!current->used && current->size >= payloadSize + MIN_BLOCK_SIZE) {
            Header *nextBlock = (Header*)((uintptr_t)current + sizeof(Header) + payloadSize);

            nextBlock->size = current->size - payloadSize - sizeof(Header);
            nextBlock->next = current->next;
            nextBlock->used = false;
            nextBlock->magic = MAGIC;

            current->size = payloadSize;
            current->next = nextBlock;
            current->used = true;
            current->magic = MAGIC;

            return GET_BLOCK(current);
        }

        if (current->next == nullptr) break;

        current = current->next;
    }

    PANIC("Heap", "Couldn't allocate block");
    return nullptr;
}

/**
 * TODO: Instead of using this I should abstract one more time
 *       It should allocate a large chunk of memory (e.g. PAGE_SIZE * 10) at a time
 *       And use a bitmap to see which pages are taken or not.
 *
 * Though I don't think this is such an issue when more is heap allocated
 */ 
void* Heap::allocAligned(size_t payloadSize, size_t alignment) {
    payloadSize = (payloadSize + (alignment - 1)) & ~(alignment - 1);

    Header* current = s_start;
    while (current && (uintptr_t)current < s_end) {
        if (current->used) {
            current = current->next;
            continue;
        }

        uintptr_t rawStart = (uintptr_t)current + sizeof(Header);
        uintptr_t aligned = (rawStart + alignment - 1) & ~(alignment - 1);
        uintptr_t alignedHeaderAddr = aligned - sizeof(Header);

        size_t prefixSize = alignedHeaderAddr - (uintptr_t)current;
        size_t totalNeeded = prefixSize + sizeof(Header) + payloadSize;

        if (current->size >= totalNeeded) {
            Header* alignedHeader = (Header*)alignedHeaderAddr;

            size_t remaining = current->size + sizeof(Header) - totalNeeded;

            if (prefixSize >= MIN_BLOCK_SIZE) {
                current->size = prefixSize - sizeof(Header);

                Header* prefixNext = alignedHeader;
                current->next = prefixNext;
            } else {
                alignedHeader = current;
                prefixSize = 0;
            }

            alignedHeader->size = payloadSize;
            alignedHeader->used = true;
            alignedHeader->magic = MAGIC;

            uintptr_t endOfAlloc = (uintptr_t)alignedHeader + sizeof(Header) + payloadSize;
            Header* suffix = (Header*)endOfAlloc;

            if (remaining >= MIN_BLOCK_SIZE) {
                suffix->size = remaining - sizeof(Header);
                suffix->used = false;
                suffix->magic = MAGIC;
                suffix->next = current->next;

                alignedHeader->next = suffix;
            } else {
                alignedHeader->next = current->next;
            }

            return GET_BLOCK(alignedHeader);
        }

        if (current->next == nullptr) break;

        current = current->next;
    }

    PANIC("Heap", "Couldn't allocate aligned block");
    return nullptr;
}

void Heap::free(void *ptr) {
    if (!ptr) return;

    Header* header = GET_HEADER(ptr);

    if (header->magic != MAGIC) PANIC("Heap", "Invalid free");


    header->used = false;

    if (header->next && !header->next->used) {
        Header* next = header->next;

        if (next->magic != MAGIC) PANIC("Heap", "Header doesn't have magic number");

        header->next = next->next;
        header->size += next->size + sizeof(Header);
    }

    Header* current = s_start;

    while (current && (uintptr_t)current < s_end) {
        if (current->next == nullptr) break;

        Header* next = current->next;

        if ((uintptr_t)next >= s_end) break;
        if (next->magic != MAGIC) PANIC("Heap", "Header doesn't have magic number");

        if (next == header) {
            if (current->used) break;

            current->size = GET_FULL_BLOCK_SIZE(header->size) + current->size;
            current->next = header->next;
            memset(header, 0, sizeof(Header));

            break;
        }

        current = next;
    }
}

void* Heap::realloc(void *ptr, size_t payloadSize) {

}