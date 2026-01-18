#include "stdio.h"

int fopen(const char* path, uint32_t mode) {
    /*int ret = syscall(SYS_open, path, mode);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }    

    return ret;*/

    char *video = (char*)0xb8000;

    const char *msg = "IT FUCKING WORKS!!\n";

    for (int i = 0; msg[i]; i++) {
        video[i*2] = msg[i];
        video[i*2+1] = 0x07;
    }

    return 1;
}

int fclose(int fd) {

}