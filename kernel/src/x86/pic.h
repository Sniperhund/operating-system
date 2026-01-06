#pragma once

#include <stdint.h>

class PIC {
public:
    /**
     * It gets enabled by default
     */
    static void remap();
    static void enable();
    static void disable();

    static void sendEOI(uint8_t irq);
};