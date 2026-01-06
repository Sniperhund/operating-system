#pragma once

#include <stddef.h>

void* memset(void* dest, int ch, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
void* memmove(void* dest, const void* src, size_t count);
int memcmp(const void* lhs, const void* rhs, size_t count);

int itoa(int value, char* buffer, int base);

size_t strlen(const char* start);
char* strrev(char* str);

char toupper(char c);