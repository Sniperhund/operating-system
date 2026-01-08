#include "drivers/text.h"
#include "panic.h"
#include "x86/gdt.h"
#include "x86/idt.h"
#include "x86/io.h"
#include "x86/irq.h"
#include "x86/memory/paging.h"
#include "x86/pic.h"
#include "drivers/keyboard.h"
#include <stdio.h>
#include "debug.h"

void timer(CPUStatus* status) {
    printf(".");
}

extern "C" void kernel_main() {
    Text::setColor(Text::BLACK, Text::LIGHT_BLUE);
    Text::init();

    PIC::disable();
    DO_INIT("Initializing GDT", GDT::init(true));
    DO_INIT("Initialising PIC", PIC::remap());
    DO_INIT("Initialising IDT", IDT::init());
    //IRQ::registerIRQ(0, timer);
    Keyboard::init(true);
    Paging::init();

    printf("D");
}