#include "exec/pid.h"
#include "exec/process.h"
#include "fs/consolefs.h"
#include "fs/fat32.h"
#include "drivers/text.h"
#include "fs/procfs.h"
#include "fs/vfs.h"
#include "sched/scheduler.h"
#include "x86/gdt.h"
#include "x86/idt.h"
#include "x86/memory/heap.h"
#include "x86/memory/pageHeap.h"
#include "x86/memory/paging.h"
#include "x86/pic.h"
#include "drivers/keyboard.h"
#include "drivers/ide.h"
#include <file.h>
#include "debug.h"
#include "syscall/syscall.h"

extern char kernel_end[];

extern "C" void kernel_main() {
    Text::setColor(Text::GRAY, Text::BLACK);
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
    VFS::mount(&ProcFS::ProcFSOps, 0, "/proc");
    VFS::mount(&ConsoleFS::ConsoleFSOps, 0, "/dev");

    inode* file;

    DO_INIT("Initialising PID", PID::init());
    DO_INIT("Initialising Scheduler", Scheduler::init());
    DO_INIT("Initialising Syscalls", Syscall::init());

    // Test program
    exec("/bin/test.elf", "");

    VFS::resolve("/proc/3/status", &file);
    printf("0x%p\n", file);
    char buf[512];

    VFS::read(file, buf, 0, sizeof(buf));

    printf("Buffer: %s\n", buf);

    Scheduler::run();
}