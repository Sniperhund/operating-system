#pragma once

#define PANIC(module, msg) \
    panic_impl((module), __FILE__, __LINE__, (msg))

[[noreturn]]
void panic_impl(const char* module, const char* file, int line, const char *msg);