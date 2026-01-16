#include "exec/elfloader.h"
#include "exec/process.h"
#include "fs/fat32.h"
#include "drivers/text.h"
#include "fs/ramfs.h"
#include "fs/vfs.h"
#include "sched/scheduler.h"
#include "x86/gdt.h"
#include "x86/idt.h"
#include "x86/memory/heap.h"
#include "x86/memory/pageHeap.h"
#include "x86/memory/paging.h"
#include "x86/pic.h"
#include "x86/irq.h"
#include "drivers/keyboard.h"
#include "drivers/ide.h"
#include <file.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "x86/syscall/syscall.h"

extern char kernel_end[];

extern "C" void kernel_main() {
    Text::setColor(Text::BLACK, Text::LIGHT_BLUE);
    Text::init();

    DO_INIT("Initializing GDT", GDT::init(true));
    DO_INIT("Initialising PIC", PIC::remap());
    DO_INIT("Initialising IDT", IDT::init());
    Keyboard::init(true);
    DO_INIT("Initialising Heap", Heap::init(kernel_end, 0xF0000));
    DO_INIT("Initialising PageHeap", PageHeap::init(32));
    DO_INIT("Initialising Paging", Paging::init());
    DO_INIT("Initialising IDE", IDE::init(0x1F0, 0x3F6, 0x170, 0x376, 0x000));
    DO_INIT("Initialising VFS", VFS::init());

    // Move to init process
    VFS::mount(&FAT32VFS::FAT32Ops, 0, "/");
    VFS::mount(&RamFS::RAMFSOps, 0, "/proc");

    DO_INIT("Initialising Scheduler", Scheduler::init());
    DO_INIT("Initialising Syscalls", Syscall::init());

    // Test program
    exec("/hello-0.elf", "");
    exec("/hello-1.elf", "");
    Scheduler::run();
}