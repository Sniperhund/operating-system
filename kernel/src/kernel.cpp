#include "drivers/text.h"
#include "panic.h"
#include "x86/gdt.h"
#include "x86/idt.h"
#include <stdio.h>

extern "C" void kernel_main() {
    Text::setColor(Text::BLACK, Text::LIGHT_BLUE);
    Text::init();
    Text::puts("Hello, world\n");

    GDT::defaultDescriptors();
    GDT::init();

    Text::puts("GDT switch\n");

    IDT::init();

    Text::puts("IDT enabled\n");
    printf("Number: %d", 12);
}