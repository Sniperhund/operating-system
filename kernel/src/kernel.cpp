#include "drivers/text.h"
#include "panic.h"
#include "x86/gdt.h"
#include "x86/idt.h"
#include "x86/io.h"
#include "x86/irq.h"
#include "x86/pic.h"
#include <stdio.h>

void timer(CPUStatus* status) {
    printf(".");
}

void keyboard(CPUStatus* status) {
    uint8_t scancode = inb(0x60);

    if (scancode & 0x80) return;

    printf("%X", scancode);
}

extern "C" void kernel_main() {
    Text::setColor(Text::BLACK, Text::LIGHT_BLUE);
    Text::init();
    Text::puts("Hello, world\n");

    GDT::defaultDescriptors();
    GDT::init();

    Text::puts("GDT switch\n");

    PIC::remap();
    IDT::init();
    IRQ::registerIRQ(0, timer);
    IRQ::registerIRQ(1, keyboard);

    Text::puts("IDT enabled\n");
    printf("Number: %d\n", 12);
}