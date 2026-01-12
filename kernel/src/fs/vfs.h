#pragma once

#include <stdint.h>
#include <stddef.h>

struct inode;

struct FSOps {
    int (*mount)(void* device, inode** root);
    int (*lookup)(inode* dir, const char* name, inode** out);
    int (*read)(inode* node, void* buffer, size_t offset, size_t size);
    int (*write)(inode* node, const void* buffer, size_t offset, size_t size);
    int (*readdir)(inode* dir, size_t index, inode** out);
    void (*destroy)(inode*);
};

struct inode {
    enum Type {
        INODE_FILE,
        INODE_DIR,
        INODE_CHARDEV,
        INODE_BLOCKDEV
    };

    Type type;
    uint32_t size;
    uint32_t refCount;

    FSOps* fs;
    void* fsData;
};

class VFS {
public:
    static int init();
    static int mount(FSOps* fs, uint8_t drive, const char* path);
    static int resolve(const char* path, inode** out);
    static size_t read(inode* node, void* buffer, size_t offset, size_t size);
    static inode* open(const char* path);
    static void close(inode* node);

private:
    struct Mount {
        const char* path;
        inode* root;
        FSOps* fs;
    };

    static Mount mounts[4];
    static size_t mountCount;
};