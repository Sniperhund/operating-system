#pragma once

#include "vfs.h"

/**
 * DISCLAIMER: This isn't "ramfs" as in linux, it is a custom memory only fs for /proc etc...
 */
class RamFS {
public:
    static FSOps RAMFSOps;

private:
    static int mount(void* device, inode** root);
    static int lookup(inode* dir, const char* name, inode** out);
    static int read(inode* node, void* buffer, size_t offset, size_t size);
    static int write(inode* node, const void* buffer, size_t offset, size_t size);
    static int readdir(inode* dir, size_t index, inode** out);
    static void destroy(inode* node);
    static void deleteE(inode* dir, inode* node);
    static int create(inode* dir, const char* name, inode** out, bool isDir);

    struct ramFSNode {
        inode* node;
        char* name;
        bool isDir;
        uint16_t childrenCount;
        // Contains a pointer to some heap allocated memory containing a dir table
        ramFSNode* children;
        // Only used for files
        uint8_t* data;
        size_t size;
    };
};