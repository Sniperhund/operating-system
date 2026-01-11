#include "fs/vfs.h"

VFS::Mount VFS::mounts[4];
size_t VFS::mountCount = 0;

int VFS::init() {
    // Intelligent!
    return 0;
}

int VFS::mount(FSOps *fs, uint8_t drive, const char *path) {
    inode* root;
    if (fs->mount((void*)(uintptr_t)drive, &root) != 0) return 1;

    mounts[mountCount++] = {
        path,
        root,
        fs
    };

    return 0;
}

int VFS::resolve(const char *path, inode **out) {
    // TODO: Resolve mount based on path
    inode* current = mounts[0].root;

    while (*path == '/') path++;

    char name[256];

    while (*path) {
        size_t len = 0;

        while (path[len] && path[len] != '/')
            name[len++] = path[len];
        name[len] = 0;

        if (current->type != inode::INODE_DIR) return 1;

        inode* next;
        if (current->fs->lookup(current, name, &next) != 0) return 1;

        current = next;
        path += len;
        while (*path == '/') path++;
    }

    *out = current;
    return 0;
}