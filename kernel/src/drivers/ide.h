#pragma once

#include <stdint.h>

class IDE {
public:
    static int init(uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3, uint32_t bar4);

private:
    struct Channel {
        uint16_t base;
        uint16_t ctrlBase;
        uint16_t bmide;     // Bus Master IDE
        uint8_t nIEN;       // No Interrupt
    };

    static Channel s_channels[2];

    struct Device {
        uint8_t reserved;       // 0 for empty, 1 for drive exists
        uint8_t channel;        // 0 for primary, 1 for secondary
        uint8_t drive;          // 0 for master, 1 for slave
        uint16_t type;          // 0 for ATA, 1 for ATAPI
        uint16_t sig;           // Signature
        uint16_t features;
        uint32_t commandSet;    // Supported command sets
        uint32_t size;          // Size in sectors
        uint8_t model[40];       // Model string
    };

    static Device s_devices[4];
    static uint8_t s_buffer[2048];
    static volatile bool s_irqInvoked;
    static uint8_t s_atapiPacket[12];

    static void ideWrite(uint8_t channel, uint8_t reg, uint8_t data);
    static uint8_t ideRead(uint8_t channel, uint8_t reg);
    static void ideReadBuffer(uint8_t channel, uint8_t reg, void* buffer, uint32_t count);
    static uint8_t idePolling(uint8_t channel, bool advancedCheck);
    static uint8_t idePrintError(uint8_t drive, uint8_t err);
};