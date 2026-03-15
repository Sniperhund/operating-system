#include "fs/consolefs.h"
#include "drivers/text.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

FSOps ConsoleFS::ConsoleFSOps = {
    .mount      = mount,
    .lookup     = lookup,
    .read       = read,
    .write      = write,
    .readdir    = nullptr,
    .destroy    = destroy,
    .deleteE    = nullptr,
    .create     = nullptr,
};

int ConsoleFS::mount(void* device, inode** root) {
    inode* node = new inode;
    node->type = inode::INODE_DIR;
    node->refCount = 1;
    node->fs = &ConsoleFSOps;

    ConsoleNode* conNode = new ConsoleNode;
    conNode->type = ConsoleNodeType::CON_ROOT;

    node->fsData = conNode;

    *root = node;
    return 0;
}

int ConsoleFS::lookup(inode* dir, const char* name, inode** out) {
    if (!dir || dir->type != inode::INODE_DIR) return -E_INVAL;

    ConsoleNode* node = (ConsoleNode*)dir->fsData;

    if (node->type != ConsoleNodeType::CON_ROOT) return -E_NOENT;

    if (strcmp(name, "stdout") == 0) {
        inode* ino = new inode;

        ino->type = inode::INODE_CHARDEV;
        ino->size = 0;
        ino->refCount = 1;
        ino->fs = &ConsoleFSOps;

        ConsoleNode* conNode = new ConsoleNode;
        conNode->type = ConsoleNodeType::CON_OUT;

        ino->fsData = conNode;
        
        *out = ino;
        return 0;
    } else if (strcmp(name, "stderr") == 0) {
        inode* ino = new inode;

        ino->type = inode::INODE_CHARDEV;
        ino->size = 0;
        ino->refCount = 1;
        ino->fs = &ConsoleFSOps;

        ConsoleNode* conNode = new ConsoleNode;
        conNode->type = ConsoleNodeType::CON_ERR;

        ino->fsData = conNode;

        *out = ino;
        return 0;
    }

    return -E_NOENT;
}

int ConsoleFS::read(inode* node, void* buffer, size_t offset, size_t size) {
    
}

int ConsoleFS::write(inode* node, const void* buffer, size_t offset, size_t size) {
    ConsoleNode* conNode = (ConsoleNode*)node->fsData;

    if (conNode->type != ConsoleNodeType::CON_OUT && conNode->type != ConsoleNodeType::CON_ERR) return -E_INVAL;

    if (conNode->type == ConsoleNodeType::CON_ERR) Text::setColor(Text::LIGHT_RED, Text::BLACK);

    const char* str = (const char*)buffer;

    for (size_t i = 0; i < size; i++) {
        Text::putc(str[i]);
    }

    if (conNode->type == ConsoleNodeType::CON_ERR) Text::setColor(Text::GRAY, Text::BLACK);

    return size;
}

void ConsoleFS::destroy(inode* node) {

}

