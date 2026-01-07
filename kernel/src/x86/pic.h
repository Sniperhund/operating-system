#pragma once

#include <stdint.h>

class PIC {
public:
    /**
     * It gets enabled by default when calling this function
     */
    static int remap();
    static void enable();
    static void disable();

    static void sendEOI(uint8_t irq);
};