#include "fs/fat32.h"
#include <string.h>

void FAT32::make83Name(const char* str, char out[11]) {
    memset(out, ' ', 11);

    int i = 0;
    while (*str && *str != '.' && i < 8)
        out[i++] = toupper(*str++);

    if (*str == '.') {
        str++;
        for (i = 8; i < 11 && *str; i++)
            out[i] = toupper(*str++);
    }
}

bool FAT32::findInDirectory(uint32_t dirCluster, const char* name, file& out) {
    char target[11];
    make83Name(name, target);

    uint32_t cluster = dirCluster;
    uint8_t buffer[512];

    while (cluster < CLUSTER_EOF) {
        uint32_t lba = clusterToLBA(cluster);

        for (uint8_t s = 0; s < bootSector->sectorsPerCluster; s++) {
            IDE::readSector(drive, lba + s, 1, buffer);

            for (int i = 0; i < 512; i += sizeof(file)) {
                file* entry = (file*)(buffer + i);

                if (entry->name[0] == 0x00) return false;
                if ((uint8_t)entry->name[0] == 0xE8) continue; // Deleted
                if (entry->attributes == 0x0F) continue; // LFN
                if (entry->attributes == 0x08) continue; // Volume label

                if (memcmp(entry->name, target, 11) == 0) {
                    out = *entry;
                    return true;
                }
            }
        }

        cluster = readFAT(cluster);
    }

    return false;
}

bool FAT32::resolvePath(const char* path, file& outEntry, uint32_t& outCluster) {
    int len = strlen(path);
    char* localPath = (char*)Heap::alloc(len);
    strcpy(localPath, path);

    char* token = strtok(localPath, "/");

    uint32_t currentCluster = rootCluster;
    file entry;

    for (int i = 0; i < 16; i++) {
        if (!findInDirectory(currentCluster, token, entry))
            return false;

        uint32_t entryCluster = entry.firstCluster;

        token = strtok(nullptr, "/");

        if (token) {
            if (!(entry.attributes & 0x10)) return false; // <-- File
            currentCluster = entryCluster;
        } else {
            outEntry = entry;
            outCluster = entryCluster;
            return true;
        }
    }

    return false;
}

uint32_t FAT32::getClusterByOffset(uint32_t startCluster, uint32_t skipClusters) {
    uint32_t cluster = startCluster;
    while (skipClusters-- && cluster < CLUSTER_EOF)
        cluster = readFAT(cluster);
    return cluster;
}

uint32_t FAT32::clusterToLBA(uint32_t cluster) {
    return firstDataSector + (cluster - 2) * bootSector->sectorsPerCluster;
}

uint32_t FAT32::readFAT(uint32_t cluster) {
    uint32_t fatOffset = cluster * 4;
    uint32_t sector = firstFatSector + (fatOffset / bootSector->bytesPerSector);
    uint32_t entryOffset = fatOffset % bootSector->bytesPerSector;

    uint8_t buffer[512]; // TODO: Change to Heap::alloc when free is implemented
    IDE::readSector(drive, sector, 1, buffer);

    uint32_t value = *(uint32_t*)(buffer + entryOffset);
    return value & 0x0FFFFFFF;
}