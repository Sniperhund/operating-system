#include "fs/ramfs.h"
#include "string.h"
#include <stdio.h>

FSOps RamFS::RAMFSOps = {
    .mount      = mount,
    .lookup     = lookup,
    .read       = read,
    .write      = write,
    .readdir    = readdir,
    .destroy    = destroy,
    .create     = create,
};

int RamFS::mount(void* device, inode** root) {
    inode* rootNode = new inode;
    rootNode->type = inode::INODE_DIR;
    rootNode->refCount = 1;
    rootNode->fs = &RAMFSOps;

    RamFSNode* ramFSNode = new RamFSNode;
    ramFSNode->node = rootNode;
    ramFSNode->name = strdup("/");
    ramFSNode->isDir = true;
    ramFSNode->childrenCount = 0;

    rootNode->fsData = ramFSNode;

    *root = rootNode;
    return 0;
}

int RamFS::lookup(inode* dir, const char* name, inode** out) {
    printf("RamFS: Lookup %s\n", name);
}

int RamFS::read(inode* node, void* buffer, size_t offset, size_t size) {
    printf("RamFS: Read %p\n", node);
}

int RamFS::write(inode* node, const void* buffer, size_t offset, size_t size) {
    printf("RamFS: Write %p\n", node);
}

int RamFS::readdir(inode* dir, size_t index, inode** out) {
    printf("RamFS: Readdir %p\n", dir);
}

void RamFS::destroy(inode* node) {
    printf("RamFS: Destroy %p\n", node);
}

int RamFS::create(inode* dir, const char* name, inode** out, bool isDir) {
    printf("RamFS: Create %s\n", name);
}
