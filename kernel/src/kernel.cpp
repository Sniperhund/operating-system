#include "drivers/text.h"
#include "panic.h"
#include "x86/gdt.h"

extern "C" void kernel_main() {
    Text::setColor(Text::BLACK, Text::LIGHT_BLUE);
    Text::init();
    Text::puts("Hello, world");

    GDT::defaultDescriptors();
    GDT::init();

    Text::puts("GDT switch");
}