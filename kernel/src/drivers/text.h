#pragma once

#include <stdint.h>

class Text {
public:
    enum Color {
        BLACK,
        BLUE,
        GREEN,
        CYAN,
        RED,
        PURPLE,
        BROWN,
        GRAY,
        DARK_GRAY,
        LIGHT_BLUE,
        LIGHT_GREEN,
        LIGHT_CYAN,
        LIGHT_RED,
        LIGHT_PURPLE,
        YELLOW,
        WHITE
    };

    static void init();
    static void putc(const char c);
    static void puts(const char* msg);
    static void setColor(Color fg, Color bg);
    static void clear();

    static void putcAt(char c, uint8_t col, uint8_t row);
    static void putsAt(const char* str, uint8_t col, uint8_t row);

private:
    static void newLine();

    static uint16_t s_color;
    static uint8_t s_col, s_row;
};