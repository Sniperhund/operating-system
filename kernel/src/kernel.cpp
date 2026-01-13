#include "exec/elfloader.h"
#include "fs/fat32.h"
#include "drivers/text.h"
#include "fs/ramfs.h"
#include "fs/vfs.h"
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

    VFS::init();

    VFS::mount(&FAT32VFS::FAT32Ops, 0, "/");
    VFS::mount(&RamFS::RAMFSOps, 0, "/proc");

    inode* file = VFS::open("/program.elf");

    if (!file) {
        printf("Couldn't open file\n");
        return;
    }

    printf("File size: %d\n", file->size);

    void* buffer = Heap::alloc(file->size);
    VFS::read(file, buffer, 0, file->size);

    ELFLoader::loadExecutable(buffer);
    printf("Program returned?");
}