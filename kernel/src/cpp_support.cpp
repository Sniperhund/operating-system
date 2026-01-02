#include <stddef.h>

extern "C" void __cxa_pure_virtual() {
    *((int*)0xb8000)=0x07690748;
}

void* operator new(size_t size) {
    return nullptr;
}

void* operator new[](size_t size) {
    return nullptr;
}

void operator delete(void* p) {
    return;
}

void operator delete[](void* p) {
    return;
}