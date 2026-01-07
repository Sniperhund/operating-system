#pragma once

class Serial {
public:
    static int init();
    static void putc(char c);
    static void puts(const char* msg);

private:
    static int isTransmitEmpty();
};