#pragma once

#include "stdarg.h"

int printf(const char* format, ...);

/**
  * Format a string with the following specifiers:
  *  d - Signed decimal integer
  *  u - Unsigned decimal integer
  *  x - Unsigned hexadecimal integer
  *  X - Unsigned hexadecimal integer (uppercase)
  *  c - Character
  *  s - String of characters
  *  p - Pointer address
  *  % - Write a single '%'
  */
int vsprintf(char* s, const char* format, va_list arg);