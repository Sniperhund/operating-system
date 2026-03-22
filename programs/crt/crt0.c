#include <stdint.h>

typedef int (*main_t)(int argc, char** argv);
extern int main(int argc, char** argv);
extern void exit(int);

void _start() {
    uint32_t* frame;
    asm volatile("movl %%ebp, %0" : "=r"(frame));

    uint32_t* oSp = frame + 1;

    int argc = (int)oSp[0];
    char** argv = (char**)&oSp[1];

    int ret = main(argc, argv);
    exit (ret);

    while (1) { }
}