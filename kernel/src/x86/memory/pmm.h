#pragma once

#include <stddef.h>
#include <stdint.h>

#define PAGE_ALIGNDOWN(addr) ((((uint32_t)(addr)) & 0xFFFFF000))

class PMM {
public:
    static int init();

    static void mark(size_t frame);
    static void unmark(size_t frame);
    static bool isMarked(size_t frame);

    static size_t findFirstFreeFrame(uint32_t amount = 1);

    /**
     * Address gets aligned down (0x4050 -> 0x4000)
     */
    static size_t physToFrame(void* phys);

private:
    static constexpr size_t bitmapSize = 2048;

    static uint32_t s_bitmap[bitmapSize];
};