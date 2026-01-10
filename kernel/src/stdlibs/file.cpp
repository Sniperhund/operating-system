#include "file.h"
#include "drivers/ide.h"
#include "panic.h"
#include "x86/memory/heap.h"
#include <string.h>
#include <stdio.h>

#define MODE_WRITE  (1 << 0)
#define MODE_READ   (1 << 1)
// If false this is a file descriptor for a drive, not a file
#define MODE_FILE   (1 << 2)

FILE* fopen(const char* path, const char* mode) {
    const char* prefix = "/drive/";
    
    if (strncmp(path, prefix, strlen(prefix)) != 0) {
        PANIC("FILE", "Only drives are supported for now");
    }

    int drive = atoi(path);

    if (drive > 4) {
        return nullptr;
    }

    FILE* file = (FILE*)Heap::alloc(sizeof(FILE));
    file->drive = drive;
    file->mode = 0; // TODO: Implement modes
    file->offset = 0;
    file->bufferSize = 256;
    file->bufferPos = 0;
    file->buffer = (uint8_t*)Heap::alloc(256);

    return file;
}

void fclose(FILE* file) {
    Heap::free(file->buffer);
    Heap::free(file);
}

size_t fwrite(FILE* file, const void* buffer, size_t count, size_t offset) {
    int prevOffset = file->offset;
    file->offset = offset;

    uint8_t* byteBuffer = (uint8_t*)buffer;

    size_t i = 0;

    for (i = 0; i < count; i++) {
        file->buffer[file->bufferPos++] = byteBuffer[i];
        
        if (file->bufferSize <= file->bufferPos) {
            fflush(file);
        }
    }

    if (file->bufferPos != 0) fflush(file);
    file->offset = prevOffset;

    return i;
}

size_t fwrite(FILE* file, const void* buffer, size_t count) {
    size_t bytesWritten = fwrite(file, buffer, count, file->offset);
    file->offset += bytesWritten;
    return bytesWritten;
}

size_t fread(FILE* file, void* buffer, size_t count, size_t offset) {
    return IDE::read(file->drive, offset, buffer, count);
}

size_t fread(FILE* file, void* buffer, size_t count) {
    size_t bytesRead = fread(file, buffer, count, file->offset);
    file->offset += bytesRead;
    return bytesRead;
}

void fflush(FILE *file) {
    // Since only drives are supported right now this is quite easy

    if (file->bufferPos == 0) return;

    IDE::write(file->drive, file->offset, file->buffer, file->bufferPos);
    file->offset += file->bufferPos;
    file->bufferPos = 0;
}