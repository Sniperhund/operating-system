#pragma once

#include <stddef.h>
#include <stdint.h>

class PageHeap {
public:
    static int init(size_t maxPages);

    /**
     * @return It gets automatically zeroed
     */
    static void* allocPage(size_t amount = 1);

    /**
     * It is particularly slow to free a page, since it needs to loop through all headers before the offending header
     * So it's O(n)
     */
    static void freePage(void* ptr);

private:
    struct Header {
        bool used;
        // Amount of pages allocated in a row.
        uint16_t sizeInPages;
        void* block;
    };

    /**
     * The first 4096 bytes of the memory is used for headers
     */
    static Header* s_first;
    static Header* s_lastAccessed;
};