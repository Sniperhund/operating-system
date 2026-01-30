ASM_SOURCE_FILES := $(shell find src -name '*.asm')
ASM_SOURCE_FILES_WITHOUT_SRC := $(patsubst src/%,dist/asm/%,$(ASM_SOURCE_FILES))
ASM_OBJECT_FILES := $(patsubst %.asm,%.o,$(ASM_SOURCE_FILES_WITHOUT_SRC))

CPP_SOURCE_FILES := $(shell find src -name '*.cpp')
CPP_SOURCE_FILES_WITHOUT_SRC := $(patsubst src/%,dist/cpp/%,$(CPP_SOURCE_FILES))
CPP_OBJECT_FILES := $(patsubst %.cpp,%.o,$(CPP_SOURCE_FILES_WITHOUT_SRC))

# Nasm compiler and flags
NASM = nasm
NASM_FLAGS = -f elf -g -F dwarf

# C compiler and flags
CPPC = i686-elf-g++
CPP_FLAGS = -g -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -Isrc -Isrc/stdlibs

override CPP_FLAGS += \
    -fno-exceptions \
	-fno-rtti

DEPFLAGS = -MMD -MP

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

dist/cpp/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CPPC) $(CPP_FLAGS) $(DEPFLAGS) -c $< -o $@

-include $(CPP_OBJECT_FILES:.o=.d)

$(KERNEL_ELF_OUTPUT): $(ASM_OBJECT_FILES) $(CPP_OBJECT_FILES)
	mkdir -p iso_root/boot

	$(LD) $(LD_FLAGS) $(ASM_OBJECT_FILES) $(CPP_OBJECT_FILES) -o $@

	grub-mkrescue -o kernel.iso iso_root

clean:
	rm -rf dist
	rm -rf iso_root/boot/kernel.elf

build: $(KERNEL_ELF_OUTPUT)

DISK_IMAGE = disk.img
DISK_SIZE = 2G

$(DISK_IMAGE):
	qemu-img create -f raw $(DISK_IMAGE) $(DISK_SIZE)

run: $(KERNEL_ELF_OUTPUT) $(DISK_IMAGE)
	qemu-system-i386 -cdrom kernel.iso -serial file:serial.log -drive format=raw,file=$(DISK_IMAGE),cache=none -boot order=dc

run-debug: $(KERNEL_ELF_OUTPUT) $(DISK_IMAGE)
	qemu-system-i386 -s -S -cdrom kernel.iso -serial file:serial.log -drive format=raw,file=$(DISK_IMAGE),cache=none -boot order=dc

# Mount disk.img as a loopback device on linux. Use at your own risk
mount-disk:
	sudo mkdir -p /mnt/mydisk
	sudo mount -o loop,rw,umask=000 $(DISK_IMAGE) /mnt/mydisk