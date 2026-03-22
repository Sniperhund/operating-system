#include "heap.h"
#include "drivers/text.h"
#include "panic.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

Heap::Header* Heap::s_start = nullptr;
uintptr_t Heap::s_end = 0;
bool Heap::s_debug = false;
Heap::DebugInfo Heap::s_debugInfo = {0, 0, 0};

/**
 * Only int 0x80 (syscalls) use the heap for now, so it's unnecessary to use `cli`
 * Though this may change when 0xE (page faults) are handled correctly since that uses the PageHeap for allocation
 * If the memory accessed from 0x80 is required to allocate a whole new page (which I think is unlikely, but you never know)
 *
 * Spinlocks should be implemented though.
 */
int Heap::init(void *start, size_t size, bool debug) {
    s_start = (Heap::Header*)start;

    s_start->size = size - sizeof(Header);
    s_start->next = nullptr;
    s_start->used = false;
    s_start->magic = MAGIC;

    s_end = (uintptr_t)start + size;

    s_debug = debug;
    s_debugInfo.used = 0;
    s_debugInfo.usedOnHeaders = sizeof(Header);
    s_debugInfo.free = s_start->size;

    return 0;
}

#define GET_HEADER(ptr) ((Heap::Header*)((uintptr_t)(ptr) - sizeof(Heap::Header)))
#define GET_BLOCK(ptr) ((void*)((uintptr_t)(ptr) + sizeof(Heap::Header)))
#define GET_FULL_BLOCK_SIZE(size) (size + sizeof(Heap::Header))

void* Heap::alloc(size_t payloadSize) {
    Header* current = s_start;

    if (current->magic != MAGIC)
        PANIC("Heap", "Corrupted header in alloc");
    

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

            if (s_debug) {
                s_debugInfo.used += current->size;
                s_debugInfo.usedOnHeaders += sizeof(Header);
                s_debugInfo.free -= current->size + sizeof(Header);
            }

            redrawDebug();
            return GET_BLOCK(current);
        }

        if (current->next == nullptr) break;

        if (current->next->magic != MAGIC) 
            PANIC("Heap", "Corrupted header in alloc");
        

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

            if (s_debug) {
                size_t memoryTaken = payloadSize;
                memoryTaken += sizeof(Header);
                if (prefixSize >= MIN_BLOCK_SIZE) memoryTaken += sizeof(Header);
                if (remaining >= MIN_BLOCK_SIZE) memoryTaken += sizeof(Header);

                s_debugInfo.used += payloadSize;
                s_debugInfo.usedOnHeaders += sizeof(Header);          // alignedHeader
                if (prefixSize >= MIN_BLOCK_SIZE) s_debugInfo.usedOnHeaders += sizeof(Header);
                if (remaining >= MIN_BLOCK_SIZE) s_debugInfo.usedOnHeaders += sizeof(Header);

                s_debugInfo.free -= memoryTaken;
            }

            redrawDebug();
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

    if (s_debug) {
        s_debugInfo.used -= header->size;
        s_debugInfo.free += header->size + sizeof(Header);
        s_debugInfo.usedOnHeaders -= sizeof(Header);
    }

    if (header->next && !header->next->used) {
        Header* next = header->next;

        if (next->magic != MAGIC) PANIC("Heap", "Header doesn't have magic number");

        header->next = next->next;
        header->size += next->size + sizeof(Header);

        if (s_debug) {
            s_debugInfo.usedOnHeaders -= sizeof(Header);
            s_debugInfo.free += sizeof(Header);
        }
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

            if (s_debug) {
                s_debugInfo.usedOnHeaders -= sizeof(Header);
                s_debugInfo.free += sizeof(Header);
            }

            break;
        }

        current = next;
    }

    redrawDebug();
}

void* Heap::realloc(void *ptr, size_t payloadSize) {

}

// TODO: Move this away and get it from text driver instead...
#define MAX_COLS 80

constexpr uint8_t DEBUG_WIDTH = 20;
constexpr uint8_t DEBUG_COL = MAX_COLS - DEBUG_WIDTH;
constexpr uint8_t DEBUG_ROW = 0;
constexpr uint8_t DEBUG_HEIGHT = 3;

void Heap::redrawDebug() {
    if (!s_debug) return;



    drawDebugBoxBorder();

    char buffer[DEBUG_WIDTH - 2];

    Text::setColor(Text::BLACK, Text::WHITE);

    sprintf(buffer, "USD: 0x%x", s_debugInfo.used);
    Text::putsAt(buffer, DEBUG_COL + 1, DEBUG_ROW + 1);
    
    sprintf(buffer, "FRE: 0x%x", s_debugInfo.free);
    Text::putsAt(buffer, DEBUG_COL + 1, DEBUG_ROW + 2);
    
    sprintf(buffer, "HDR: 0x%x", s_debugInfo.usedOnHeaders);
    Text::putsAt(buffer, DEBUG_COL + 1, DEBUG_ROW + 3);

    Text::setColor(Text::WHITE, Text::BLACK);

    uint32_t expected = s_end - (uintptr_t)s_start;
    uint32_t actual = s_debugInfo.used + s_debugInfo.free + s_debugInfo.usedOnHeaders;

    if (actual != expected) {
        PANIC("Heap", "Accounting mismatch");
    }
}

void Heap::drawDebugBoxBorder() {
    Text::setColor(Text::BLACK, Text::WHITE);

    // Top
    Text::putcAt('+', DEBUG_COL, DEBUG_ROW);
    for (int i = 1; i < DEBUG_WIDTH - 1; i++)
        Text::putcAt('-', DEBUG_COL + i, DEBUG_ROW);
    Text::putcAt('+', DEBUG_COL + DEBUG_WIDTH - 1, DEBUG_ROW);

    // Bottom
    Text::putcAt('+', DEBUG_COL, DEBUG_ROW + DEBUG_HEIGHT + 1);
    for (int i = 1; i < DEBUG_WIDTH - 1; i++)
        Text::putcAt('-', DEBUG_COL + i, DEBUG_ROW + DEBUG_HEIGHT + 1);
    Text::putcAt('+', DEBUG_COL + DEBUG_WIDTH - 1, DEBUG_ROW + DEBUG_HEIGHT + 1);

    // Vertical
    for (int row = DEBUG_ROW + 1; row <= DEBUG_ROW + DEBUG_HEIGHT; row++) {
        Text::putcAt('|', DEBUG_COL, row);
        Text::putcAt('|', DEBUG_COL + DEBUG_WIDTH - 1, row);
    }

    // Inside
    for (int row = DEBUG_ROW + 1; row <= DEBUG_ROW + DEBUG_HEIGHT; row++) {
        for (int col = 1; col <= DEBUG_WIDTH - 2; col++) {
            Text::putcAt(' ', DEBUG_COL + col, row);
        }
    }

    Text::setColor(Text::WHITE, Text::BLACK);
}