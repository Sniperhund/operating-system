#pragma once

#include "sched/scheduler.h"
#include "vfs.h"

class ConsoleFS {
public:
    static FSOps ConsoleFSOps;

private:
    static int mount(void* device, inode** root);
    static int lookup(inode* dir, const char* name, inode** out);
    static int read(inode* node, void* buffer, size_t offset, size_t size);
    static int write(inode* node, const void* buffer, size_t offset, size_t size);
    static void destroy(inode* node);

    enum ConsoleNodeType {
        CON_ROOT,
        CON_OUT,
        CON_ERR,
        CON_IN
    };

    struct ConsoleNode {
        ConsoleNodeType type;
    };

    static WaitQueue s_stdinQueue;
};
