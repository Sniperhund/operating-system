#pragma once

#include "drivers/ide.h"
#include "x86/memory/heap.h"
#include <stdint.h>
#include <stdio.h>

class FAT32 {
public:
    FAT32(uint8_t drive);

    void walkRootDir();

    struct inode {
        char name[8];
        char ext[3];
        uint8_t attributes;
        uint8_t reserved;
        uint8_t createTimeMS;
        uint16_t createTime;
        uint16_t createDate;
        uint16_t lastAccessDate;
        uint16_t eaIndex;
        uint16_t lastModifiedTime;
        uint16_t lastModifiedDate;
        uint16_t firstCluster;
        uint32_t size; // In bytes
    } __attribute__((packed));

    bool resolvePath(const char* path, inode& outEntry, uint32_t& outCluster);

    /**
     * Read a file from a entry
     *
     * @return Bytes read
     */
    size_t readFile(const inode& entry, void* buffer, size_t offset, size_t count);
    /**
     * Write to a file from a buffer
     * Permissions are not yet supported.
     * 
     * @return Bytes written
     */
    size_t writeFile(const char* path, void* buffer, inode& out, size_t offset, size_t count);

private:
    struct BPB {
        uint8_t reserved0[3];
        char oem[8];
        uint16_t bytesPerSector;
        uint8_t sectorsPerCluster;
        uint16_t reservedSectorCount; // Number of reserved sectors at the start of the parition. Boot record/MBR are included
        uint8_t tableCount;
        uint16_t rootEntryCount;
        uint16_t totalSectors16;
        uint8_t media; // Media Descriptor Type
        uint16_t tableSize16; // FAT/FAT16 only
        uint16_t sectorsPerTrack;
        uint16_t headSideCount;
        uint32_t hiddenSectorCount;
        uint32_t totalSectors32;

        uint32_t tableSize32;
        uint16_t flags;
        uint16_t version;
        uint32_t rootCluster;
        uint16_t FSInfo;
        uint16_t backupBootSector;
        uint8_t reserved1[12];
        uint8_t driveNum;
        uint8_t windowsNTFlags;
        uint8_t sig; // Must be either 0x28 or 0x29
        uint32_t reserved3; // Volume ID 'Serial' number. This can be ignored and is.
        char label[11];
        char identifier[8];
    } __attribute__((packed));

    struct FSInfo {
        uint32_t loadSig;   // Must be 0x41615252
        uint8_t reserved0[480];
        uint32_t sig;       // Must be 0x61417272
        uint32_t lastKnownFreeClusterCount; // If the value is 0xFFFFFFFF the count is unknown and must be computed.
        uint32_t nextFreeClusterHint;       // If the value is 0XFFFFFFFF it's unknown
        uint8_t reserved1[12];
        uint32_t tailSig;   // Must be 0xAA550000
    } __attribute__((packed));

    struct LongFileName {
        uint8_t order;
        char char0[10]; // First 2-byte characters
        uint8_t attributes;
        uint8_t type;
        uint8_t checksum;
        char char1[12];
        uint16_t reserved;
        char char2[4];
    } __attribute__((packed));

    uint32_t totalSectors = 0;
    uint32_t size = 0;
    uint32_t firstFatSector = 0;
    uint32_t firstDataSector = 0;
    uint32_t dataSectorCount = 0;
    uint32_t totalClusters = 0;

    uint32_t rootCluster = 0;

    uint8_t type = 0;
    uint8_t drive = 0;

    BPB* bootSector = nullptr;

    static constexpr uint32_t CLUSTER_EOF = 0x0FFFFFF8;

    uint32_t clusterToLBA(uint32_t cluster);
    /**
     * Get the next cluster in the chain
     */
    uint32_t readFAT(uint32_t cluster);
    uint32_t getClusterByOffset(uint32_t startCluster, uint32_t skipClusters);

    void make83Name(const char* str, char out[11]);

    bool findInDirectory(uint32_t dirCluster, const char* name, inode& out);
};