#pragma once

#define DO_INIT(msg, func) \
    do { \
        int status = (func); \
        if (status == 0) printf("%s: DONE\n", msg); \
        else printf("%s (%d): FAILED\n", msg, status); \
    } while (0)

#define BOCHS_BREAK asm volatile ("xchgw %bx, %bx");

__attribute__((noinline)) void loadDebugSymbols(const char* path, unsigned int textBase);