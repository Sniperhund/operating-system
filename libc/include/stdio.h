#pragma once

#include <stdint.h>

int fopen(const char* path, uint32_t mode);
int fclose(int fd);