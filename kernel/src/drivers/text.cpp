#include "text.h"
#include <string.h>
#include "debug.h"
#include "serial.h"
#include <stdio.h>

volatile uint16_t* video = (volatile uint16_t*)0xB8000;

#define MAX_COLS 80
#define MAX_ROWS 25

uint16_t Text::s_color = 0x07;
uint8_t Text::s_col = 0, Text::s_row = 0;

void Text::init() {
    clear();
    DO_INIT("Initialising Serial", Serial::init());
}

void Text::putc(const char c) {
    if (c == '\n') {
        newLine();
        return;
    }

    video[s_row * MAX_COLS + s_col] = c | (s_color << 8);

    s_col++;

    if (s_col == MAX_COLS) {
        newLine();
    }
}

void Text::puts(const char *msg) {
    while (*msg != 0) {
        putc(*msg++);
    }
}

void Text::setColor(Color fg, Color bg) {
    s_color = ((bg & 0xF) << 4) | (fg & 0xF);
}

void Text::clear() {
    s_col = 0;
    s_row = 0;

    for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        video[i] = ' ' | (s_color << 8);
    }
}

void Text::putcAt(char c, uint8_t col, uint8_t row) {
    if (col >= MAX_COLS || row >= MAX_ROWS) return;

    video[row * MAX_COLS + col] = c | (s_color << 8);
}

void Text::putsAt(const char* str, uint8_t col, uint8_t row) {
    while (*str && col < MAX_COLS) {
        putcAt(*str++, col++, row);
    }
}

void Text::newLine() {
    s_row++;
    s_col = 0;

    if (s_row >= MAX_ROWS) {
        s_row = MAX_ROWS - 1;

        memmove((void*)video, (void*)(video + MAX_COLS), (MAX_ROWS - 1) * MAX_COLS * sizeof(uint16_t));
        
        uint16_t blank = ' ' | (s_color << 8);
        volatile uint16_t* row = video + (MAX_ROWS - 1) * MAX_COLS;

        for (size_t i = 0; i < MAX_COLS; i++) {
            row[i] = blank;
        }
    }
}