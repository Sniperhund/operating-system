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
                    for (int j = 0; j < len; j++) s[k++] = buffer[j];
                    break;
                }
            }
        }
    }

    s[k] = '\0';
    return k;
}
