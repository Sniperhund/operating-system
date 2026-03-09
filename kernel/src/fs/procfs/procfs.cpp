#include "fs/procfs.h"
#include "error.h"
#include "exec/process.h"
#include "sched/scheduler.h"
#include <string.h>
#include <stdio.h>

FSOps ProcFS::ProcFSOps = {
    .mount      = mount,
    .lookup     = lookup,
    .read       = read,
    .write      = nullptr,
    .readdir    = nullptr,
    .destroy    = destroy,
    .deleteE    = nullptr,
    .create     = nullptr,
};

int ProcFS::mount(void* device, inode** root) {
    inode* rootNode = new inode;
    rootNode->type = inode::INODE_DIR;
    rootNode->refCount = 1;
    rootNode->fs = &ProcFSOps;

    ProcNode* procNode = new ProcNode;
    procNode->type = PROC_ROOT;
    procNode->proc = nullptr;

    rootNode->fsData = procNode;

    *root = rootNode;
    return 0;
}

int ProcFS::lookup(inode* dir, const char* name, inode** out) {
    ProcNode* procNode = (ProcNode*)dir->fsData;

    if (procNode->type == PROC_ROOT) {
        pid_t pid = atoi(name);
        if (pid > 0) {
            Proc* proc = Scheduler::getByPid(pid);
            if (!proc) return -E_NOENT;

            inode* node = new inode;
            node->type = inode::INODE_DIR;
            node->refCount = 1;
            node->fs = &ProcFSOps;

            ProcNode* newProcNode = new ProcNode;
            newProcNode->type = PROC_PID_DIR;
            newProcNode->proc = proc;
            node->fsData = newProcNode;

            *out = node;
            return 0;
        }
    } else if (procNode->type == PROC_PID_DIR) {
        if (strcmp(name, "status") == 0) {
            inode* node = new inode;
            node->type = inode::INODE_FILE;
            node->refCount = 1;
            node->fs = &ProcFSOps;

            ProcNode* newProcNode = new ProcNode;
            newProcNode->type = PROC_PID_STATUS;
            newProcNode->proc = procNode->proc;
            node->fsData = newProcNode;

            *out = node;
            return 0;
        }
    }

    return -E_NOENT;
}

int ProcFS::read(inode* node, void* buffer, size_t offset, size_t size) {
    ProcNode* procNode = (ProcNode*)node->fsData;
    if (!procNode || !procNode->proc) return 0;

    char tmp[512];
    size_t len = 0;

    Proc* proc = procNode->proc;

    switch (procNode->type) {
        case PROC_PID_STATUS:
            len = sprintf(tmp, "Pid: %d\nState: %s\n",
                proc->pid,
                proc->stateString());
            break;
        default:
            return 0;
    }

    if (offset >= len) return 0;
    size_t toCopy = len - offset < size ? size : len - offset;
    strncpy((char*)buffer, tmp + offset, toCopy);
    return toCopy;
}

void ProcFS::destroy(inode* node) {

}