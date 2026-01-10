#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Rewrite to be a full VFS instead.
 * 
 * That VFS should be able to read both real files, full drives and in-memory file descriptors (like stdout)
 */

struct FILE {
    uint8_t mode;
    uint32_t offset;
    // TODO: Update this with a inode
    uint8_t drive;

    // If bufferSize == 0 the buffer is not used
    uint32_t bufferSize;
    uint32_t bufferPos;
    uint8_t* buffer;
};

FILE* fopen(const char* path, const char* mode);
void fclose(FILE* file);

/**
 * Write to the file at a specific offset, the offset in the file descriptor will not be updated
 */
size_t fwrite(FILE* file, const void* buffer, size_t count, size_t offset);

/**
 * Write to the file at the current offset
 */
size_t fwrite(FILE* file, const void* buffer, size_t count);

/**
 * Read from the file at a specific offset, the offset in the file descriptor will not be updated
 */
size_t fread(FILE* file, void* buffer, size_t count, size_t offset);

/**
 * Read from the file at the current offset
 */
size_t fread(FILE* file, void* buffer, size_t count);

/**
 * Set the offset of a file descriptor
 */
void foffset(FILE* file, size_t offset);

/**
 * Flush the buffer to disk
 * fflush advances file->offset automatically
 */
void fflush(FILE* file);