#include "stdio.h"
#include "drivers/text.h"
#include "string.h"

int printf(const char* format, ...) {
    va_list arg;
    va_start(arg, format);

    char buffer[1024];

    int ret = vsprintf(buffer, format, arg);

    Text::puts(buffer);

    va_end(arg);

    return ret;
}

// Very illegal macro, but I don't care
#define PRINT_BUFFER for (int j = 0; j < len; j++) s[k++] = buffer[j]

int vsprintf(char* s, const char* format, va_list args) {
    int k = 0;

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] != '%') {
            s[k++] = format[i];
        } else {
            i++;
            char buffer[32]; // Temporary buffer for numbers
            int len = 0;

            switch (format[i]) {
                case 'd': {
                    int val = va_arg(args, int);
                    len = itoa(val, buffer, 10);
                    PRINT_BUFFER;
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    len = itoa(val, buffer, 10);
                    PRINT_BUFFER;
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    len = itoa(val, buffer, 16);
                    PRINT_BUFFER;
                    break;
                }
                case 'X': {
                    unsigned val = va_arg(args, unsigned int);
                    len = itoa(val, buffer, 16);
                    for (int j = 0; j < len; j++) buffer[j] = toupper(buffer[j]);
                    PRINT_BUFFER;
                    break;
                }
                case 'c': {
                    char val = va_arg(args, unsigned int);
                    s[k++] = val;
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    len = strlen(str);
                    for (int j = 0; j < len; j++) s[k++] = str[j];
                    break;
                }
                case 'p': {
                    uintptr_t ptr = va_arg(args, uintptr_t);
                    len = itoa(ptr, buffer, 16);
                    PRINT_BUFFER;
                    break;
                }
                case 'P': {
                    uintptr_t ptr = va_arg(args, uintptr_t);
                    len = itoa(ptr, buffer, 16);
                    for (int j = 0; j < len; j++) buffer[j] = toupper(buffer[j]);
                    PRINT_BUFFER;
                    break;
                }
                case '%': {
                    s[k++] = '%';
                }
            }
        }
    }

    s[k] = '\0';
    return k;
}
