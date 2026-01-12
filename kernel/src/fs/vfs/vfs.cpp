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

size_t VFS::read(inode* node, void* buffer, size_t offset, size_t size) {
    if (!node || !buffer) return 0;

    if (node->type != inode::INODE_FILE) return 0;
    if (!node->fs || !node->fs->read) return 0;

    return node->fs->read(node, buffer, offset, size);
}

inode* VFS::open(const char *path) {
    inode* current = mounts[0].root;

    while (*path == '/') path++;

    char name[256];

    while (*path) {
        size_t len = 0;

        if (current->type != inode::INODE_DIR) return nullptr;

        while (path[len] && path[len] != '/') name[len++] = path[len];
        name[len] = 0;

        inode* next;
        if (current->fs->lookup(current, name, &next) != 0) return nullptr;

        current = next;
        path += len;
        while (*path == '/') path++;
    }

    return current;
}

void VFS::close(inode *node) {
    if (!node) return;
    if (--node->refCount > 0) return;

    if (node->fs && node->fs->destroy)
        node->fs->destroy(node);

    delete node;
}