#pragma once

#include "bitmap.h"
#include <sys/types.h>

class PID {
public:
    static int init();

    static pid_t allocate();
    static void deallocate(pid_t pid);

private:
    static pid_t s_lastPid;
    static Bitmap s_bitmap;

    static constexpr uint32_t MAX_PID = 32 * 1024;
};