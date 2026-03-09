#include "pid.h"
#include <error.h>

pid_t PID::s_lastPid;
Bitmap PID::s_bitmap;

int PID::init() {
    s_lastPid = 2;
    int result = s_bitmap.init(MAX_PID);

    if (result) return result;

    s_bitmap.mark(0);
    s_bitmap.mark(1);

    return 0;
}

pid_t PID::allocate() {
    pid_t pid = s_lastPid + 1;
    if (pid > MAX_PID)
        pid = 2;

    pid_t start = pid;

    while (true) {
        if (!s_bitmap.isMarked(pid)) {
            s_bitmap.mark(pid);
            s_lastPid = pid;
            return pid;
        }

        pid++;
        if (pid > MAX_PID) pid = 2;
        if (pid == start) return -E_PID;
    }
}

void PID::deallocate(pid_t pid) {
    if (pid < 2 || pid > MAX_PID)
        return;
    s_bitmap.unmark(pid);
}
