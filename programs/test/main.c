#include "stdint.h"
#include "stdio.h"
#include <unistd.h>

void _start() {
    int ret = fopen("/test.txt", 0);

    if (ret == -1) exit(255);
    exit(ret);

    exit(0);
}
