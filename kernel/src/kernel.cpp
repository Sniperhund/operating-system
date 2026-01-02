#include "drivers/text.h"

extern "C" void kernel_main() {
    Text::setColor(Text::BLACK, Text::LIGHT_BLUE);
    Text::init();
    Text::puts("Hello, world");
}