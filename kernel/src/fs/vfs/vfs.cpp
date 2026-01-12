#include "fs/vfs.h"
#include <string.h>

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
    if (!out || !*path) return 1;

    inode* current = nullptr;
    FSOps* fs = nullptr;

    size_t bestLen = 0;
    for (size_t i = 0; i < mountCount; i++) {
        const char* mountPath = mounts[i].path;

        size_t len = strlen(mountPath);
        if (len == 0) continue;

        if (len == 1) {
            if (bestLen == 0) {
                bestLen = 1;
                current = mounts[i].root;
                fs = mounts[i].fs;
            }
        }

        if (strncmp(path, mountPath, len) == 0 && (path[len] == '/' || path[len] == 0)) {
            if (len > bestLen) {
                bestLen = len;
                current = mounts[i].root;
                fs = mounts[i].fs;
            }
        }
    }

    if (!current) return 1;

    path += bestLen;
    while (*path == '/') path++;

    char name[256];
    while (*path) {
        size_t len = 0;
        while (path[len] && path[len] != '/') name[len++] = path[len];
        name[len] = 0;

        if (current->type != inode::INODE_DIR) return 1;

        inode* next;
        if (!current->fs || current->fs->lookup(current, name, &next) != 0) return 1;
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

size_t VFS::write(inode *node, void *buffer, size_t offset, size_t size) {
    if (!node || !buffer) return 0;

    if (node->type != inode::INODE_FILE) return 0;
    if (!node->fs || !node->fs->write) return 0;

    return node->fs->write(node, buffer, offset, size);
}

inode* VFS::open(const char *path, uint32_t flags) {
    inode* node = nullptr;

    if(resolve(path, &node) == 0) return node;

    if (!(flags & O_CREATE)) return nullptr;

    char parentPath[256], name[256];
    splitPath(path, parentPath, name);

    inode* parent = nullptr;
    if (resolve(parentPath, &parent) != 0) return nullptr;
    if (!parent->fs || !parent->fs->create) return nullptr;

    inode* newNode = nullptr;
    if (parent->fs->create(parent, name, &newNode, false) != 0)
        return nullptr;

    newNode->flags = flags;

    return newNode;
    
}

void VFS::close(inode *node) {
    if (!node) return;
    if (--node->refCount > 0) return;

    if (node->fs && node->fs->destroy)
        node->fs->destroy(node);

    delete node;
}