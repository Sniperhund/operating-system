#include "stdint.h"
#include "stdio.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

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

    for (int i = 0; i < 128; i++) {
        buffer[i] = 0;
    }

    fread(0, buffer, 128, 0);
    printf("[%d]: %s", errno, buffer);

    return 1;
}
    
