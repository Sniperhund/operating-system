#include "fat32.h"
#include "panic.h"
#include "x86/memory/heap.h"
#include <string.h>

FAT32::FAT32(uint8_t drive) {
    this->drive = drive;
    bootSector = (BPB*)Heap::alloc(512);

    IDE::readSector(drive, 0, 1, bootSector);

    char label[12] = {0};
    strncpy(label, bootSector->label, 11);

    printf("Reading FAT32: %s\n", label);

    totalSectors = bootSector->totalSectors16 ? bootSector->totalSectors16 : bootSector->totalSectors32;
    size = bootSector->tableSize16 ? bootSector->tableSize16 : bootSector->tableSize32;
    firstFatSector = bootSector->reservedSectorCount;
    firstDataSector = bootSector->reservedSectorCount + (bootSector->tableCount * size);
    dataSectorCount = totalSectors - firstDataSector;
    totalClusters = dataSectorCount / bootSector->sectorsPerCluster;

    if (totalClusters < 4085) type = 12;
    else if (totalClusters < 65525) type = 16;
    else type = 32;

    if (type != 32) PANIC("FAT32", "Only FAT32 is supported");

    rootCluster = bootSector->rootCluster;
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

uint32_t FAT32::getClusterByOffset(uint32_t startCluster, uint32_t skipClusters) {
    uint32_t cluster = startCluster;
    while (skipClusters-- && cluster < CLUSTER_EOF)
        cluster = readFAT(cluster);
    return cluster;
}

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

bool FAT32::findInDirectory(uint32_t dirCluster, const char* name, inode& out) {
    char target[11];
    make83Name(name, target);

    uint32_t cluster = dirCluster;
    uint8_t buffer[512];

    while (cluster < CLUSTER_EOF) {
        uint32_t lba = clusterToLBA(cluster);

        for (uint8_t s = 0; s < bootSector->sectorsPerCluster; s++) {
            IDE::readSector(drive, lba + s, 1, buffer);

            for (int i = 0; i < 512; i += sizeof(inode)) {
                inode* entry = (inode*)(buffer + i);

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

bool FAT32::resolvePath(const char* path, inode& outEntry, uint32_t& outCluster) {
    int len = strlen(path);
    char* localPath = (char*)Heap::alloc(len);
    strcpy(localPath, path);

    char* token = strtok(localPath, "/");

    uint32_t currentCluster = rootCluster;
    inode entry;

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

size_t FAT32::readFile(const inode& entry, void* buffer, size_t offset, size_t count) {
    if (offset >= entry.size || !buffer)
        return 0;

    size_t bytesPerCluster = bootSector->bytesPerSector * bootSector->sectorsPerCluster;

    if (offset + count > entry.size)
        count = entry.size - offset;

    uint32_t startCluster = entry.firstCluster;

    size_t clusterIndex = offset / bytesPerCluster;
    uint32_t clusterOffset = offset % bytesPerCluster;

    uint32_t cluster = getClusterByOffset(startCluster, clusterIndex);

    uint8_t sectorBuffer[512];
    size_t bytesRead = 0;
    uint8_t* out = (uint8_t*)buffer;

    while (cluster < CLUSTER_EOF && bytesRead < count) {
        uint32_t lba = clusterToLBA(cluster);

        for (uint8_t s = 0; s < bootSector->sectorsPerCluster && bytesRead < count; s++) {
            IDE::readSector(drive, lba + s, 1, sectorBuffer);

            size_t sectorOffset = (clusterOffset > 0) ? clusterOffset : 0;

            size_t bytesToCopy = bootSector->bytesPerSector - sectorOffset;

            if (bytesToCopy > count - bytesRead)
                bytesToCopy = count - bytesRead;

            memcpy(out + bytesRead, sectorBuffer + sectorOffset, bytesToCopy);

            bytesRead += bytesToCopy;
            clusterOffset = 0;
        }

        cluster = readFAT(cluster);
    }

    return bytesRead;
}

