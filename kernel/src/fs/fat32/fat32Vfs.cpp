#include "fs/fat32.h"
#include "fs/vfs.h"

int FAT32VFS::mount(void* device, inode **outRoot) {
    uint8_t drive = (uint8_t)(uintptr_t)device;
    FAT32* fs = new FAT32(drive);

    fat32Node* rootNode = new fat32Node;
    rootNode->fs = fs;
    rootNode->cluster = fs->getRootCluster();
    rootNode->isDir = true;

    inode* root = new inode;
    root->type = inode::INODE_DIR;
    root->size = 0;
    root->refCount = 1;
    root->fs = &FAT32Ops;
    root->fsData = rootNode;

    *outRoot = root;
    return 0;
}

int FAT32VFS::lookup(inode* dir, const char* name, inode** out) {
    fat32Node* node = (fat32Node*)dir->fsData;
    FAT32* fs = node->fs;

    FAT32::file entry;
    if (!fs->findInDirectory(node->cluster, name, entry))
        return -1;

    fat32Node* child = new fat32Node;
    child->fs = fs;
    child->cluster = entry.firstCluster;
    child->isDir = (entry.attributes & 0x10);

    inode* ino = new inode;
    ino->type = child->isDir ? inode::INODE_DIR : inode::INODE_FILE;
    ino->size = entry.size;
    ino->refCount = 1;
    ino->fs = &FAT32Ops;
    ino->fsData = child;

    *out = ino;
    return 0;
}

int FAT32VFS::read(inode* node, void* buffer, size_t offset, size_t size) {
    fat32Node* fatNode = (fat32Node*)node->fsData;

    FAT32::file fake;
    fake.firstCluster = fatNode->cluster;
    fake.size = node->size;

    return fatNode->fs->readFile(fake, buffer, offset, size);
}

void FAT32VFS::destroy(inode *node) {
    fat32Node* fatNode = (fat32Node*)node->fsData;
    delete fatNode;
}

FSOps FAT32VFS::FAT32Ops = {
    .mount      = mount,
    .lookup     = lookup,
    .read       = read,
    .write      = nullptr,
    .readdir    = nullptr,
    .destroy    = destroy,
};