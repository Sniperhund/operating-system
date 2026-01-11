#include "fs/fat32.h"
#include <string.h>

void FAT32::walkRootDir() {
    uint32_t cluster = rootCluster;

    uint8_t buffer[512]; // TODO: Change to Heap::alloc when free is implemented

    while (cluster < CLUSTER_EOF) {
        uint32_t lba = clusterToLBA(cluster);

        for (uint8_t s = 0; s < bootSector->sectorsPerCluster; s++) {
            IDE::readSector(drive, lba + s, 1, buffer);

            for (int i = 0; i < 512; i += sizeof(inode)) {
                inode* entry = (inode*)(buffer + i);

                if (entry->name[0] == 0x0) return;
                if ((uint8_t)entry->name[0] == 0xE5) continue;
                if (entry->attributes == 0x0F) continue; // LFN
                if (entry->attributes & 0x08) continue;  // Volume label

                char name[13];
                strncpy(name, entry->name, 8);
                name[8] = '.';
                strncpy(name + 9, entry->ext, 3);
                name[12] = 0;

                printf("FILE: %s  size:%d\n", name, entry->size);
            }
        }

        cluster = readFAT(cluster);
    }
}