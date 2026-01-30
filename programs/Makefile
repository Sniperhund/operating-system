TARGET   := i686-elf
CC       := $(TARGET)-gcc

ROOT     := $(CURDIR)
OUT      := $(ROOT)/binaries
SYSROOT  := $(ROOT)/sysroot
LIBC_SRC := $(ROOT)/../libc
LIBC_BUILD := $(LIBC_SRC)/build/lib/libc.a
LIBC_A := $(SYSROOT)/lib/libc.a

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

$(LIBC_BUILD):
	@echo "[*] Building libc..."
	$(MAKE) -C $(LIBC_SRC)

$(LIBC_A): $(LIBC_BUILD)
	@echo "[*] Installing libc into sysroot"
	mkdir -p $(SYSROOT)/include $(SYSROOT)/lib
	cp -r $(LIBC_SRC)/include/* $(SYSROOT)/include/
	cp $(LIBC_BUILD) $(SYSROOT)/lib/

$(OUT)/%.elf: %/main.c $(LIBC_A)
	@echo "[*] Building $*"
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(SYSROOT)/lib/libc.a -o $@

all: $(ELFS)

clean:
	rm -rf $(OUT) $(SYSROOT)