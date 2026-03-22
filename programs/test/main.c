#include "stdint.h"
#include "stdio.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    printf("Hello, world\n");
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    char procFile[64] = {0};

    sprintf(procFile, "/proc/%d/status", getpid());

    char buffer[128] = {0};

    int fd = fopen(procFile, 0);
    fread(fd, buffer, 128, 0);
    printf("%s", buffer);

    printf("Running exec\n");
    exec("/bin/test.elf", "hello, usermode 2");
    printf("exec returned\n");

    return 1;
}
    
