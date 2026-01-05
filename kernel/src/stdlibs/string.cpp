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

    for (size_t i = 0; i < count; i++) {
        destPtr[i] = srcPtr[i];
    }

    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    uint8_t* destPtr = (uint8_t*)dest;
    const uint8_t* srcPtr = (const uint8_t*)src;

    if (destPtr == srcPtr || count == 0) return dest;

    if (destPtr < srcPtr || destPtr >= srcPtr + count) {
        // No overlap, copy forwards
        for (size_t i = 0; i < count; i++) {
            destPtr[i] = srcPtr[i];
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

}