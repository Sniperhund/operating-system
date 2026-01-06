#pragma once

#include "x86/idt.h"
#include <stdint.h>

extern "C" void irqHandlerC(CPUStatus* status);

class IRQ {
public:
    typedef void (*irqHandlerFunc) (CPUStatus*);

    static int registerIRQ(uint8_t irq, irqHandlerFunc func);
private:
    friend void ::irqHandlerC(CPUStatus* status);

    static void irqHandler(CPUStatus* status);

    static irqHandlerFunc s_irqRoutines[16];
};
