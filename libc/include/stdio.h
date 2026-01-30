#pragma once

#include <stdint.h>
#include <stddef.h>

int fread(int fd, char* buf, size_t count, size_t offset);
int fwrite(int fd, const char* buf, size_t count, size_t offset);
int fopen(const char* path, uint32_t mode);
int fclose(int fd);

void perror(const char* s);