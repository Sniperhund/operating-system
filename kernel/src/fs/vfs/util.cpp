#include "fs/vfs.h"
#include "string.h"

void VFS::splitPath(const char* path, char* parent, char* name) {
    const char* lastSlash = strrchr(path, '/');
    if (!lastSlash) {
        strcpy(parent, "/");
        strcpy(name, path);
    } else if (lastSlash == path) {
        strcpy(parent, "/");
        strcpy(name, lastSlash + 1);
    } else {
        size_t len = lastSlash - path;
        strncpy(parent, path, len);
        parent[len] = 0;
        strcpy(name, lastSlash + 1);
    }
}