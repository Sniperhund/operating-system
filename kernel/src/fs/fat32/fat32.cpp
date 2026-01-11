#include "fs/fat32.h"
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