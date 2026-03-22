#include "syscall.h"
#include "unistd.h"

void exec(const char* file, const char* argv) {
    SYSCALL2(7, file, argv);
}