#include "pageHeap.h"
#include "panic.h"
#include "x86/memory/heap.h"
#include "x86/memory/paging.h"
#include <stdio.h>
#include <string.h>

PageHeap::Header* PageHeap::s_first = nullptr;
PageHeap::Header* PageHeap::s_lastAccessed = nullptr;
size_t PageHeap::s_maxPages = 0;

#define NEXT_HEADER(header) ((PageHeap::Header*)((uintptr_t)(header) + sizeof(PageHeap::Header)))
#define GET_HEADER(header) ((PageHeap::Header*)(s_first + (((uintptr_t)(header) - (uintptr_t)s_first) / PAGE_SIZE - 1)))

int PageHeap::init(size_t maxPages) {
    s_first = (Header*)Heap::allocAligned(PAGE_SIZE * (1 + maxPages), PAGE_SIZE);
    s_lastAccessed = s_first;

    memset(s_first, 0, PAGE_SIZE * (1 + maxPages));

    uintptr_t startBlock = (uintptr_t)s_first + PAGE_SIZE;
    Header* current = s_first;
    for (int i = 0; i < maxPages; i++) {
        current->block = (void*)((uintptr_t)startBlock + PAGE_SIZE * i);
        current->used = false;
        current->sizeInPages = 1;

        current = NEXT_HEADER(current);
    }

    s_maxPages = maxPages;

    return 0;
}

void* PageHeap::allocPage(size_t amount) {
    if (amount == 0) return nullptr;

    Header* current = nullptr;
    if (s_lastAccessed == nullptr) current = s_first;
    else current = s_lastAccessed;
    
    Header* candidate = nullptr;
    size_t freeCount = 0;

    while (current && (uintptr_t)current < (uintptr_t)s_first + PAGE_SIZE) {
        if (!current->used) {
            if (candidate == nullptr) candidate = current;
            freeCount++;

            if (freeCount >= amount) {
                Header* h = candidate;
                for (size_t i = 0; i < amount; i++) {
                    h->used = true;
                    h->sizeInPages = amount;
                    h = NEXT_HEADER(h);
                }

                s_lastAccessed = candidate;
                return candidate->block;
            }
        } else {
            candidate = nullptr;
            freeCount = 0;
        }

        current = NEXT_HEADER(current);
        if (current->block == nullptr) {
            if (s_lastAccessed != nullptr) {
                current = s_first;
                s_lastAccessed = nullptr;
                candidate = nullptr;
                freeCount = 0;
                continue;
            } else break;
        }
    }

    return nullptr;
}

void PageHeap::freePage(void *ptr) {
    if (!ptr) return;

    if ((uintptr_t)ptr % PAGE_SIZE != 0)
        PANIC("PageHeap", "Unaligned pointer");

    Header* header = getHeaderFromPtr(ptr);

    if (!header->used) return;

    size_t size = header->sizeInPages;

    for (size_t i = 0; i < size; i++) {
        header->used = false;
        header = NEXT_HEADER(header);
    }

    s_lastAccessed = getHeaderFromPtr(ptr);
}

PageHeap::Header* PageHeap::getHeaderFromPtr(void* ptr) {
    uintptr_t startBlock = (uintptr_t)s_first + PAGE_SIZE;
    size_t index = ((uintptr_t)ptr - startBlock) / PAGE_SIZE;
    return (Header*)((uintptr_t)s_first + index * sizeof(Header));
}

bool PageHeap::isManaged(void *ptr) {
  uintptr_t start = (uintptr_t)s_first + PAGE_SIZE;
  uintptr_t end = start + (s_maxPages * PAGE_SIZE);
  return (uintptr_t)ptr >= start && (uintptr_t)ptr < end;
}
