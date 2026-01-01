ASM_SOURCE_FILES := $(shell find src -name '*.asm')
ASM_SOURCE_FILES_WITHOUT_SRC := $(patsubst src/%,dist/asm/%,$(ASM_SOURCE_FILES))
ASM_OBJECT_FILES := $(patsubst %.asm,%.o,$(ASM_SOURCE_FILES_WITHOUT_SRC))

C_SOURCE_FILES := $(shell find src -name '*.c')
C_SOURCE_FILES_WITHOUT_SRC := $(patsubst src/%,dist/c/%,$(C_SOURCE_FILES))
C_OBJECT_FILES := $(patsubst %.c,%.o,$(C_SOURCE_FILES_WITHOUT_SRC))

# Nasm compiler and flags
NASM = nasm
NASM_FLAGS = -f elf -g -F dwarf

# C compiler and flags
CC = i686-elf-gcc
C_FLAGS = -g -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -Isrc -Isrc/stdlibs

# Linker and flags
LD = i686-elf-ld
LD_FLAGS =

override LD_FLAGS += \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    -T linker.ld

KERNEL_ELF_OUTPUT = iso_root/boot/kernel.elf

dist/asm/%.o: src/%.asm
	mkdir -p $(dir $@)
	$(NASM) $(NASM_FLAGS) $< -o $@

dist/c/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

# Toolchain for building the 'limine' executable for the host.
HOST_CC := cc
HOST_CFLAGS := -g -O2 -pipe
HOST_LDFLAGS :=
HOST_LIBS :=

$(KERNEL_ELF_OUTPUT): $(ASM_OBJECT_FILES) $(C_OBJECT_FILES)
	mkdir -p iso_root/boot

	$(LD) $(LD_FLAGS) $(ASM_OBJECT_FILES) $(C_OBJECT_FILES) -o $@

	grub-mkrescue -o kernel.iso iso_root

clean:
	rm -rf dist
	rm -rf iso_root/boot/kernel.elf

build: $(KERNEL_ELF_OUTPUT)

run: $(KERNEL_ELF_OUTPUT)
	qemu-system-x86_64 -cdrom kernel.iso

run-debug: $(KERNEL_ELF_OUTPUT)
	qemu-system-x86_64 -s -S -cdrom kernel.iso
