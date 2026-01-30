#pragma once

#include "stdarg.h"

int printf(const char* format, ...);
int sprintf(char* s, const char* format, ...);

/**
  * Format a string with the following specifiers:
  * @li d - Signed decimal integer
  * @li u - Unsigned decimal integer
  * @li x - Unsigned hexadecimal integer
  * @li X - Unsigned hexadecimal integer (uppercase)
  * @li b - Unsigned bit integer
  * @li c - Character
  * @li s - String of characters
  * @li p - Pointer address
  * @li % - Write a single '%'
  */
int vsprintf(char* s, const char* format, va_list arg);