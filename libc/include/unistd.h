#pragma once

#include "sys/types.h"

int exit(int status);
pid_t getpid();
pid_t getppid();

void exec(const char* file, const char* argv);