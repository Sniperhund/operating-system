#include "stdint.h"
#include "stdio.h"

void _start() {
    fopen("", 0);

    asm volatile("mov $4, %eax; int $0x80");
}