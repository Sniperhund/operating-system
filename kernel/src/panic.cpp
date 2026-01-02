#include "panic.h"

#include "drivers/text.h"

void panic(const char* module, const char *msg) {
    Text::puts("\nPANIC (");
    Text::puts(module);
    Text::puts("): ");
    Text::puts(msg);

    while (1) {
        asm volatile("cli; hlt");
    }
}