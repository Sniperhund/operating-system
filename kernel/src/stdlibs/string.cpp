#include "string.h"

#include <stdint.h>

void* memset(void* dest, int value, size_t count) {
    uint8_t* ptrByte = (uint8_t*)dest;

    for (size_t i = 0; i < count; i++) {
        ptrByte[i] = (uint8_t)value;
    }

    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    uint8_t* destPtr = (uint8_t*)dest, *srcPtr = (uint8_t*)src;

    if (destPtr == srcPtr || count == 0) return dest;

    while (count--) {
        *destPtr++ = *srcPtr++;
    }

    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    uint8_t* destPtr = (uint8_t*)dest;
    const uint8_t* srcPtr = (const uint8_t*)src;

    if (destPtr == srcPtr || count == 0) return dest;

    if (destPtr < srcPtr || destPtr >= srcPtr + count) {
        // No overlap, copy forwards
        while (count--) {
            *destPtr++ = *srcPtr++;
        }
    } else {
        // Overlap, copy backwards
        for (size_t i = count; i > 0; i--) {
            destPtr[i - 1] = srcPtr[i - 1];
        }
    }

    return dest;
}

int memcmp(const void* lhs, const void* rhs, size_t count) {
    const uint8_t *a = (const uint8_t *)lhs;
    const uint8_t *b = (const uint8_t *)rhs;

    for (size_t i = 0; i < count; i++) {
        if (a[i] != b[i]) {
            return (int)a[i] - (int)b[i];
        }
    }
    return 0;
}


int itoa(int value, char *buffer, int base) {
    char digits[] = "0123456789abcdef";
    int i = 0;
    bool negative = false;

    if (value == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return i;
    }

    if (base == 10 && value < 0) {
        negative = true;
        value = -value;
    }

    while (value != 0) {
        int remainder = value % base;
        buffer[i++] = digits[remainder];
        value /= base;
    }

    if (negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';
    
    strrev(buffer);
    return i;
}

int utoa(unsigned int value, char *buffer, int base) {
    char digits[] = "0123456789abcdef";
    int i = 0;

    if (value == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return i;
    }

    while (value != 0) {
        buffer[i++] = digits[value % base];
        value /= base;
    }

    buffer[i] = '\0';
    strrev(buffer);
    return i;
}

size_t strlen(const char *start) {
    const char* end = start;
    while (*end != '\0')
        ++end;
    return end - start;
}

char* strrev(char* str) {
    if (!str || !*str) {
        return str;
    }

    int i = strlen(str) - 1, j = 0;
    char ch;
    
    while (i > j) {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }

    return str;
}

char* strcpy(char* dest, const char* src) {
    int len = strlen(src);

    memmove(dest, src, len);

    return dest;
}

int strcmp(const char* lhs, const char* rhs) {
    int lhsLen = strlen(lhs);
    int rhsLen = strlen(rhs);

    int len = lhsLen >= rhsLen ? lhsLen : rhsLen;

    return strncmp(lhs, rhs, len);
}

int strncmp(const char* lhs, const char* rhs, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (lhs[i] != rhs[i]) {
            return (int)lhs[i] - (int)rhs[i];
        }
    }

    return 0;
}

int atoi(const char *str) {
    int num = 0;
    bool negative = false;
    
    while (*str != '\0') {
        if (*str == '-' && ((*(str + 1) >= '0') && (*(str + 1) <= '9'))) {
            negative = true;
            str++;
        }

        if (!((*str >= '0') && (*str <= '9'))) {
            str++;
            continue;
        }

        num = num * 10;
        num = num + (*str - 48);
        str++;
    }

    return num * (negative ? -1 : 1);
}

char toupper(char c) {
    const char offset = 'a' - 'A';
    return (c >= 'a' && c <= 'z') ? c -= offset : c;
}