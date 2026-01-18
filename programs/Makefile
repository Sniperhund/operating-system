TARGET   := i686-elf
CC       := $(TARGET)-gcc

ROOT     := $(CURDIR)
OUT      := $(ROOT)/binaries
SYSROOT  := $(ROOT)/sysroot
LIBC_SRC := $(ROOT)/../libc

CFLAGS := \
	-ffreestanding \
	-fno-builtin \
	-fno-stack-protector \
	-nostdlib \
	-static \
	-I$(SYSROOT)/include

LDFLAGS := -T linker.ld

# Every directory except binaries/ and sysroot/
ALL_DIRS := $(wildcard */)
ALL_DIRS := $(ALL_DIRS:/=)
PROGRAM_DIRS := $(filter-out sysroot binaries,$(ALL_DIRS))
ELFS := $(PROGRAM_DIRS:%=$(OUT)/%.elf)

all: sysroot $(ELFS)

sysroot:
	@echo "[*] Installing libc into sysroot"
	rm -rf $(SYSROOT)
	mkdir -p $(SYSROOT)/include $(SYSROOT)/lib
	cp -r $(LIBC_SRC)/include/* $(SYSROOT)/include/
	cp $(LIBC_SRC)/build/lib/libc.a $(SYSROOT)/lib/

$(OUT)/%.elf: %/main.c
	@echo "[*] Building $*"
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(SYSROOT)/lib/libc.a -o $@

all: sysroot $(ELFS)

clean:
	rm -rf $(OUT) $(SYSROOT)