#pragma once

#include "exec/process.h"
#include "vfs.h"

class ProcFS {
public:
    static FSOps ProcFSOps;

private:
    static int mount(void* device, inode** root);
    static int lookup(inode* dir, const char* name, inode** out);
    static int read(inode* node, void* buffer, size_t offset, size_t size);
    static void destroy(inode* node);

    enum ProcNodeType {
        PROC_ROOT,
        PROC_PID_DIR,
        PROC_PID_STATUS
    };

    struct ProcNode {
        ProcNodeType type;
        Proc* proc;
    };
};