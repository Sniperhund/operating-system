#pragma once

#define SYSCALL0(nr) \
    ({  int r; \
        asm volatile("int $0x80" \
            : "=a"(r) \
            : "a"(nr) \
            : "memory"); \
        r; })

#define SYSCALL1(nr, a) \
    ({  int r; \
        asm volatile("int $0x80" \
            : "=a"(r) \
            : "a"(nr), "b"(a) \
            : "memory"); \
        r; })

#define SYSCALL2(nr, a, b) \
    ({  int r; \
        asm volatile("int $0x80" \
            : "=a"(r) \
            : "a"(nr), "b"(a), "c"(b) \
            : "memory"); \
        r; })

#define SYSCALL3(nr, a, b, c) \
    ({  int r; \
        asm volatile("int $0x80" \
            : "=a"(r) \
            : "a"(nr), "b"(a), "c"(b), "d"(c) \
            : "memory"); \
        r; })

#define SYSCALL4(nr, a, b, c, d) \
    ({  int r; \
        asm volatile("int $0x80" \
            : "=a"(r) \
            : "a"(nr), "b"(a), "c"(b), "d"(c), "S"(d) \
            : "memory"); \
        r; })

extern int errno;

static inline int __syscall_ret(int ret) {
    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return ret;
}