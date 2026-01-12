#include "irq.h"
#include "x86/pic.h"
#include <stdio.h>

IRQ::irqHandlerFunc IRQ::s_irqRoutines[16];

int IRQ::registerIRQ(uint8_t irq, irqHandlerFunc func) {
    if (!func) return 1;
    
    s_irqRoutines[irq] = func;

    return 0;
}

void IRQ::irqHandler(CPUStatus* status) {
    if (s_irqRoutines[status->intNo]) s_irqRoutines[status->intNo](status);

    PIC::sendEOI(status->intNo);
}

void irqHandlerC(CPUStatus* status) {
    IRQ::irqHandler(status);
}
