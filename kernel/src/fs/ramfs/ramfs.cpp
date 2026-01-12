#include "fs/ramfs.h"
#include "string.h"
#include "x86/memory/heap.h"
#include <stdio.h>
#include <error.h>

FSOps RamFS::RAMFSOps = {
    .mount      = mount,
    .lookup     = lookup,
    .read       = read,
    .write      = write,
    .readdir    = readdir,
    .destroy    = destroy,
    .deleteE    = deleteE,
    .create     = create,
};

int RamFS::mount(void* device, inode** root) {
    inode* rootNode = new inode;
    rootNode->type = inode::INODE_DIR;
    rootNode->refCount = 1;
    rootNode->fs = &RAMFSOps;

    ramFSNode* ramNode = new ramFSNode;
    ramNode->node = rootNode;
    ramNode->name = strdup("/");
    ramNode->isDir = true;
    ramNode->childrenCount = 0;
    ramNode->children = nullptr;

    rootNode->fsData = ramNode;

    *root = rootNode;
    return 0;
}

int RamFS::lookup(inode* dir, const char* name, inode** out) {
    ramFSNode* node = (ramFSNode*)dir->fsData;
    
    for (int i = 0; i < node->childrenCount; i++) {
        ramFSNode* child = node->children + i;

        if (strcmp(name, child->name) == 0) {
            if (child->node) {
                *out = child->node;
                child->node->refCount++;
                return 0;
            }

            inode* ino = new inode;
            ino->type = child->isDir ? inode::INODE_DIR : inode::INODE_FILE;
            ino->size = child->size;
            ino->refCount = 1;
            ino->fs = &RAMFSOps;
            ino->fsData = child;
            child->node = ino;
            
            *out = ino;
            return 0;
        }
    }

<<<<<<< HEAD
    return E_NOENT;
}

int RamFS::read(inode* node, void* buffer, size_t offset, size_t size) {
    // TODO: Set a error bit somewhere
=======
    return 1;
}

int RamFS::read(inode* node, void* buffer, size_t offset, size_t size) {
>>>>>>> 3149168ee81a07d25c3a140852298f68df61e568
    if (!node) return 0;

    ramFSNode* fsNode = (ramFSNode*)node->fsData;
    
    if (offset >= fsNode->size) return 0;

    if (offset + size > fsNode->size)
        size = fsNode->size - offset;

    memcpy(buffer, fsNode->data + offset, size);

    return size;
}

int RamFS::write(inode* node, const void* buffer, size_t offset, size_t size) {
<<<<<<< HEAD
    // TODO: Set a error bit somewhere
=======
>>>>>>> 3149168ee81a07d25c3a140852298f68df61e568
    if (!node) return 0;

    ramFSNode* fsNode = (ramFSNode*)node->fsData;

    size_t offsetSize = offset + size;

    if (fsNode->size == 0) {
        fsNode->size = offsetSize;
        node->size = offsetSize;
        fsNode->data = (uint8_t*)Heap::alloc(offsetSize);
    } else if (offsetSize > fsNode->size) {
        fsNode->data = (uint8_t*)Heap::realloc(fsNode->data, offsetSize);
        memset(fsNode->data + fsNode->size, 0, offset - fsNode->size);
        fsNode->size = offsetSize;
        node->size = offsetSize;
    }

    memcpy(fsNode->data + offset, buffer, size);

    return size;
}

int RamFS::readdir(inode* dir, size_t index, inode** out) {
<<<<<<< HEAD
    if (!dir) return E_INVAL;

    ramFSNode* node = (ramFSNode*)dir->fsData;

    if (index >= node->childrenCount) return E_NOENT;

=======
    if (!dir) return 1;

    ramFSNode* node = (ramFSNode*)dir->fsData;

>>>>>>> 3149168ee81a07d25c3a140852298f68df61e568
    ramFSNode* child = node->children + index;

    if (child->node) {
        *out = child->node;
        child->node->refCount++;
        return 0;
    }

    inode* ino = new inode;
    ino->type = child->isDir ? inode::INODE_DIR : inode::INODE_FILE;
    ino->size = child->size;
    ino->refCount = 1;
    ino->fs = &RAMFSOps;
    ino->fsData = child;
    child->node = ino;

    *out = ino;
    return 0;
}

void RamFS::destroy(inode* node) {
    ramFSNode* fsNode = (ramFSNode*)node->fsData;

    if (fsNode && fsNode->node == node)
        fsNode->node = nullptr;

    delete node;
}

void RamFS::deleteE(inode* node) {

}

int RamFS::create(inode* dir, const char* name, inode** out, bool isDir) {
    ramFSNode* node = (ramFSNode*)dir->fsData;

    for (int i = 0; i < node->childrenCount; i++) {
        ramFSNode* child = node->children + i;
<<<<<<< HEAD
        if (strcmp(name, child->name) == 0) return E_NOTUNIQ;
=======
        if (strcmp(name, child->name) == 0) return 1;
>>>>>>> 3149168ee81a07d25c3a140852298f68df61e568
    }

    ramFSNode* newNode = nullptr;

    if (node->childrenCount == 0) {
        node->childrenCount++;
        node->children = (ramFSNode*)Heap::alloc(sizeof(ramFSNode));
        newNode = node->children;
    } else {
        node->childrenCount++;
        node->children = (ramFSNode*)Heap::realloc(node->children, node->childrenCount * sizeof(ramFSNode));
        newNode = &node->children[node->childrenCount - 1];
    }

    inode* ino = new inode;

    newNode->node = ino;
    newNode->isDir = isDir;
    newNode->name = strdup(name);
    newNode->childrenCount = 0;
    newNode->children = nullptr;
    newNode->size = 0;

    ino->size = 0;
    ino->type = isDir ? inode::INODE_DIR : inode::INODE_FILE;
    ino->fs = &RAMFSOps;
    ino->refCount = 1;
    ino->fsData = newNode;
    
    *out = ino;
    return 0;
}
