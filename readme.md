# Operating System

This is my very bad OS, it's made for x86 with multiboot<br>
It is made with C++ (don't judge me, too much atleast) and therefore in freestanding, so no STL, exceptions or RTTI.<br>

It doesn't really follow any standard, but takes a lot of inspiration from Linux (and therefore the POSIX standard)

The code in most places aren't very good (I think so atleast), but it works _(mostly)_

### Features

The features implemented are as follows:

-   Interrupts (Exceptions and IRQs, though a syscall interrupt is WIP)
-   Paging (not fully with permissions and other flags)
-   TSS with the ability to jump into usermode (though I'm working on a better implementation of usermode)
-   Kernel heap
-   printf for the Kernel
-   A few libc functions
-   FAT32 and VFS (only reading for now)

## Building

You will need:

-   Make
-   i386-elf tools
-   nasm
-   grub (Multiboot2 compat)
-   QEMU (for Makefile, though this can be substited with Bochs or an emulator of your choosing)

To build `kernel.iso`, you can simply run:

```
cd kernel
make build
```

To run the kernel with QEMU, you can run:

```
cd kernel
make run
```

If you want to use the VFS implementation you have to format `kernel/disk.img` with FAT32 yourself<br>
On linux you can use the following:

```
mkfs.vfat -F 32 disk.img
```

This may very well not work on your system, as I have not tested it except on my system<br>
Feel free to open a issue if you run into problems and I'll try my best to help

### Issues and feedback

This is my first _real_ attempt at building a kernel (I usually quit when paging failed for 15th time lol)<br>
If you have any feedback or any issues arise for you, I'll be happy to help if you open a issue :)
