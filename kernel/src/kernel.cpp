#include "fs/fat32.h"
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

    FAT32 fat(0);

    FAT32::inode file;
    uint32_t cluster;

    if (fat.resolvePath("/test.txt", file, cluster)) {
        printf("Found file. Size: %u, startCluster: %u\n", file.size, cluster);

        char* buffer = (char*)Heap::alloc(file.size);

        fat.readFile(file, buffer, 0, file.size);

        printf("Buffer: %s", buffer);
    }
}