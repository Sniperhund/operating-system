#include "panic.h"

#include "stdio.h"

void panic_impl(const char* module, const char* file, int line, const char *msg) {
    printf("\n=== KERNEL PANIC ===\n");
    printf("[%s] %s:%d\n", module, file, line);
    printf("%s\n", msg);

    while (1) {
        asm volatile("cli; hlt");
    }
}