# Source directories
LIBC_SRC := ./src
LIBC_INCLUDE := ./include

# Output directories
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
LIB_DIR := $(BUILD_DIR)/lib

# C compiler and flags
CC := i686-elf-gcc
CFLAGS := -g -Wall -Wextra -ffreestanding -nostdlib -nostartfiles \
          -Isrc -I$(LIBC_INCLUDE) -fno-builtin -fno-stack-protector

# Target static library
LIBC_A := $(LIB_DIR)/libc.a

# Find all C source files in libc
C_SOURCE_FILES := $(shell find $(LIBC_SRC) -name '*.c')
OBJ_FILES := $(patsubst $(LIBC_SRC)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCE_FILES))

# Default target
all: $(LIBC_A)

# Compile libc .c files into object files
$(OBJ_DIR)/%.o: $(LIBC_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Archive all object files into libc.a
$(LIBC_A): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	ar rcs $@ $(OBJ_FILES)
	@echo "[✓] Built libc: $@"

# Clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
