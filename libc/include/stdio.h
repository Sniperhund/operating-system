#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

int fread(int fd, char* buf, size_t count, size_t offset);
int fwrite(int fd, const char* buf, size_t count, size_t offset);
int fopen(const char* path, uint32_t mode);
int fclose(int fd);

void perror(const char* s);

int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int vsprintf(char* s, const char* format, va_list args);