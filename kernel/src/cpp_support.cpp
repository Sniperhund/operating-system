#include <stddef.h>
#include "x86/memory/heap.h"
#include "error.h"

extern "C" void __cxa_pure_virtual() {

}

void* operator new(size_t size) {
    return Heap::alloc(size);
}

void* operator new[](size_t size) {
    return Heap::alloc(size);
}

void operator delete(void* p) {
    Heap::free(p);
}

void operator delete[](void* p) {
    Heap::free(p);
}

void operator delete(void* p, unsigned long size) {
    Heap::free(p);
}