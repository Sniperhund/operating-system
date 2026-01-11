#include "fs/fat32.h"
#include <string.h>

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

