#include "serial.h"
#include "x86/io.h"

#define PORT 0x3F8

int Serial::init() {
    outb(PORT + 1, 0);
    outb(PORT + 3, 0x80);
    outb(PORT, 0x03);
    outb(PORT + 1, 0);
    outb(PORT + 3, 0x03);
    outb(PORT + 2, 0xC7);
    outb(PORT + 4, 0x0B);
    outb(PORT + 4, 0x1E);
    outb(PORT, 0xAE);

    if (inb(PORT) != 0xAE) {
        return 1;
    }

    outb(PORT + 4, 0x0F);
    return 0;
}

void Serial::putc(char c) {
    while (isTransmitEmpty() == 0);

    outb(PORT, c);
}

void Serial::puts(const char *msg) {
    while (*msg != 0) {
        putc(*msg++);
    }
}

int Serial::isTransmitEmpty() {
    return inb(PORT + 5) & 0x20;
}