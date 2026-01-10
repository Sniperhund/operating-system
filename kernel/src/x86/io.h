#pragma once

#include <stdint.h>
#include <stddef.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void insb(uint16_t port, uint8_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        buffer[i] = inb(port);
    }
}

static inline void outsb(uint16_t port, uint8_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        outb(port, buffer[i]);
    }
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inb %w1, %w0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile("outw %w0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void insw(uint16_t port, uint16_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        buffer[i] = inw(port);
    }
}

static inline void outsw(uint16_t port, uint16_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        outw(port, buffer[i]);
    }
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %w1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void insl(uint16_t port, uint32_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        buffer[i] = inl(port);
    }
}

static inline void outsl(uint16_t port, uint32_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        outl(port, buffer[i]);
    }
}

static inline void ioWait() {
    outb(0x80, 0);
}