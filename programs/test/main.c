#include "stdint.h"
#include "stdio.h"
#include <unistd.h>

void _start() {
    int ret = fopen("/test.txt", 0);

    if (ret == -1) exit(255);

    char buffer[6] = {0};
    fread(ret, (void*)buffer, 5, 0);
    buffer[5] = 0;
    fwrite(0, buffer, 6, 0);

    exit(0);
}
