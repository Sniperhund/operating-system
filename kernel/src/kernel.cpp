#include "drivers/text.h"
#include "x86/gdt.h"
#include "x86/idt.h"
#include "x86/memory/heap.h"
#include "x86/memory/pageHeap.h"
#include "x86/memory/paging.h"
#include "x86/pic.h"
#include "drivers/keyboard.h"
#include "drivers/ide.h"
#include <file.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

void timer(CPUStatus* status) {
    printf(".");
}

extern char kernel_end[];

extern "C" void kernel_main() {
    Text::setColor(Text::BLACK, Text::LIGHT_BLUE);
    Text::init();

    PIC::disable();
    DO_INIT("Initializing GDT", GDT::init(true));
    DO_INIT("Initialising PIC", PIC::remap());
    DO_INIT("Initialising IDT", IDT::init());
    //IRQ::registerIRQ(0, timer);
    Keyboard::init(true);
    DO_INIT("Initialising Heap", Heap::init(kernel_end, 0xF0000));
    DO_INIT("Initialising PageHeap", PageHeap::init(16));
    DO_INIT("Initialising Paging", Paging::init());
    DO_INIT("Initialising IDE", IDE::init(0x1F0, 0x3F6, 0x170, 0x376, 0x000));

    FILE* file = fopen("/drive/0", "");

    printf("0x%p, %d\n", file);

    uint32_t* buffer = (uint32_t*)Heap::alloc(4);
    memset(buffer, 0xFF, 4);

    fwrite(file, buffer, 4);

    printf("%u - ", *buffer);

    memset(buffer, 0, 4);

    fread(file, buffer, 4, file->offset - 4);

    printf("%u", *buffer);
}