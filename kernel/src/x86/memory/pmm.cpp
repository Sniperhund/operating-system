#include "pmm.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>

uint32_t PMM::s_bitmap[bitmapSize];

#define BITS_PER_WORD (sizeof(uint32_t) * CHAR_BIT)
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

int PMM::init() {
    memset(s_bitmap, 0, bitmapSize * sizeof(uint32_t));
}

void PMM::mark(size_t n) {
    s_bitmap[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void PMM::unmark(size_t n) {
    s_bitmap[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

bool PMM::isMarked(size_t n) {
    uint32_t bit = s_bitmap[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return bit != 0;
}

size_t PMM::findFirstFreeFrame(uint32_t amount) {
    size_t consecutive = 0;  // How many free frames we've found in a row
    size_t start = 0;        // Start index of the current free run

    for (size_t i = 0; i < bitmapSize * BITS_PER_WORD; i++) {
        if (!isMarked(i)) {
            if (consecutive == 0) {
                start = i; // Potential start of a run
            }
            consecutive++;

            if (consecutive == amount) {
                return start;
            }
        } else {
            consecutive = 0; // Reset run if frame is occupied
        }
    }

    return -1; // Not enough consecutive free frames found
}

size_t PMM::physToFrame(void *phys) {
    return PAGE_ALIGNDOWN(phys) >> 12;
}