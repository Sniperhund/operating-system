#include "debug.h"
#include <stdio.h>
#include <drivers/serial.h>

__attribute__((noinline))
void loadDebugSymbols(const char *path, unsigned int textBase) {
    char buf[256];
    sprintf(buf, "\nSYMLOAD:%s:0x%x", path, textBase);
    Serial::puts(buf);

    volatile unsigned int anchor = textBase;
    (void)anchor;
}