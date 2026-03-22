# Operating System

This is my very bad OS, it's made for x86 with multiboot<br>
It is made with C++ (don't judge me, too much atleast) and therefore in freestanding, so no STL, exceptions or RTTI.<br>

It doesn't really follow any standard, but takes a lot of inspiration from Linux (and therefore the POSIX standard)

The code in most places aren't very good (I think so atleast), but it works _(mostly)_

### Features

The features implemented are as follows:

- Interrupts (Exceptions and IRQs)
- Paging (now with permissions)
- TSS
- Kernel heap
- printf for the Kernel
- A few libc functions for the kernel
- FAT32 (only reading for now)
- RamFS (reading and writing)
- ProcFS (with status file only for now)
- ConsoleFS (for writting to stdout in usermode). Though this will likely be merged into ProcFS at some point.
- VFS
- ELF loader (only for executables)
- Preemptive scheduler (round robin)
- Usermode
- Syscalls for fs operations
- File libc function for usermode

## Building

You will need:

- Make
- i386-elf tools
- nasm
- grub (Multiboot2 compat)
- QEMU (for Makefile, though this can be substited with Bochs or an emulator of your choosing)

To build `kernel.iso`, you can simply run:

```
cd kernel
make build
```

## Running

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

## Helper script

I have written a small helper script (`run.sh`) that automate building and running (for the kernel and user programs).

You can use that, but be sure to read it beforehand since it automatically mounts a loopback disk.<br>
And you have to format `kernel/disk.img` with FAT32 beforehand manually.

### Issues and feedback

This is my first _real_ attempt at building a kernel (I usually quit when paging failed for 15th time lol)<br>
If you have any feedback or any issues arise for you, I'll be happy to help if you open a issue :)
