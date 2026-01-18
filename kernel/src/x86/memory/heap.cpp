#include "heap.h"
#include "panic.h"
#include <stdio.h>

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

            current->size = payloadSize;
            current->next = nextBlock;
            current->used = true;

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
        uintptr_t aligned = ((uintptr_t)current + alignment - 1) & ~(alignment - 1);
        size_t paddingRequired = aligned - (uintptr_t)current;
        size_t paddedPayloadSize = paddingRequired + payloadSize;

        if (!current->used && current->size >= paddedPayloadSize + MIN_BLOCK_SIZE) {
            Header* nextBlock = (Header*)((uintptr_t)current + sizeof(Header) + paddedPayloadSize);

            nextBlock->size = current->size - paddedPayloadSize - sizeof(Header);
            nextBlock->next = current->next;
            nextBlock->used = false;

            current->size = paddedPayloadSize;
            current->next = nextBlock;
            current->used = true;

            // NOTE: This may be a problem later if a header is aligns perfectly with alignment
            //       Then it may return the header instead of the block
            return (void*)aligned;
        }

        if (current->next == nullptr) break;

        current = current->next;
    }

    PANIC("Heap", "Couldn't allocate block");
    return nullptr;
}

void Heap::free(void *ptr) {
    Header* header = GET_HEADER(ptr);
    printf("Free was called for %u bytes\n", header->size);
}

void* Heap::realloc(void *ptr, size_t payloadSize) {

}