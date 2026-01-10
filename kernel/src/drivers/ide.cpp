#include "ide.h"
#include "x86/io.h"
#include <stdio.h>

// Command/Status Port Returns
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Features/Error
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

// Commands
#define ATA_CMD_READ_PIO            0x20
#define ATA_CMD_READ_PIO_EXT        0x24
#define ATA_CMD_READ_DMA            0xC8
#define ATA_CMD_READ_DMA_EXT        0x25
#define ATA_CMD_WRITE_PIO           0x30
#define ATA_CMD_WRITE_PIO_EXT       0x34
#define ATA_CMD_WRITE_DMA           0xCA
#define ATA_CMD_WRITE_DMA_EXT       0x35
#define ATA_CMD_CACHE_FLUSH         0xE7
#define ATA_CMD_CACHE_FLUSH_EXT     0xEA
#define ATA_CMD_PACKET              0xA0
#define ATA_CMD_IDENTIFY_PACKET     0xA1
#define ATA_CMD_IDENTIFY            0xEC

// ATAPI specific Commands
#define ATAPI_CMD_READ              0xA8
#define ATAPI_CMD_EJECT             0x1B

// Identify Command Returns
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

// Drive Selection
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

// ALTSTATUS/CONTROL Returns
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

IDE::Channel IDE::s_channels[2];
IDE::Device IDE::s_devices[4];

uint8_t IDE::s_buffer[2048] = {0};
volatile bool IDE::s_irqInvoked = false;
uint8_t IDE::s_atapiPacket[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int IDE::init(uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3, uint32_t bar4) {
    int k, count = 0;

    // Detect IO ports
    s_channels[ATA_PRIMARY  ].base      = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
    s_channels[ATA_PRIMARY  ].ctrlBase  = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
    s_channels[ATA_SECONDARY].base      = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
    s_channels[ATA_SECONDARY].ctrlBase  = (bar3 & 0xFFFFFFFC) + 0x376 * (!bar3);
    s_channels[ATA_PRIMARY  ].bmide     = (bar4 & 0xFFFFFFFC) + 0; // Bus Master IDE
    s_channels[ATA_SECONDARY].bmide     = (bar4 & 0xFFFFFFFC) + 8; // Bus Master IDE

    // Disable IRQs
    ideWrite(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
    ideWrite(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++) {
            unsigned char err = 0, type = IDE_ATA, status;
            s_devices[count].reserved = 0; // Assuming that no drive here.

            ideWrite(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
            for (int m = 0; m < 16; m++) { asm volatile("nop"); }
            //sleep(1);

            ideWrite(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            for (int m = 0; m < 16; m++) { asm volatile("nop"); }
            //sleep(1);

            if (ideRead(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.

            while(1) {
                status = ideRead(i, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR)) {
                    // If Err, Device is not ATA.
                    err = 1;
                    break;
                }

                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
            }

            // Probe for ATAPI Devices:
            if (err != 0) {
                unsigned char cl = ideRead(i, ATA_REG_LBA1);
                unsigned char ch = ideRead(i, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB)
                type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                type = IDE_ATAPI;
                else
                continue; // Unknown Type (may not be a device).

                ideWrite(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                for (int m = 0; m < 16; m++) { asm volatile("nop"); }
                //sleep(1);
            }

            // (V) Read Identification Space of the Device:
            ideReadBuffer(i, ATA_REG_DATA, (void*)s_buffer, 128);

            // (VI) Read Device Parameters:
            s_devices[count].reserved     = 1;
            s_devices[count].type         = type;
            s_devices[count].channel      = i;
            s_devices[count].drive        = j;
            s_devices[count].sig    = *((unsigned short *)(s_buffer + ATA_IDENT_DEVICETYPE));
            s_devices[count].features = *((unsigned short *)(s_buffer + ATA_IDENT_CAPABILITIES));
            s_devices[count].commandSet  = *((unsigned int *)(s_buffer + ATA_IDENT_COMMANDSETS));

            // (VII) Get Size:
            if (s_devices[count].commandSet & (1 << 26))
                // Device uses 48-Bit Addressing:
                s_devices[count].size   = *((unsigned int *)(s_buffer + ATA_IDENT_MAX_LBA_EXT));
            else
                // Device uses CHS or 28-bit Addressing:
                s_devices[count].size   = *((unsigned int *)(s_buffer + ATA_IDENT_MAX_LBA));

            // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
            for(k = 0; k < 40; k += 2) {
                s_devices[count].model[k] = s_buffer[ATA_IDENT_MODEL + k + 1];
                s_devices[count].model[k + 1] = s_buffer[ATA_IDENT_MODEL + k];}
            s_devices[count].model[40] = 0; // Terminate String.

            count++;
      }

    // 4- Print Summary:
    for (int i = 0; i < 4; i++)
        if (s_devices[i].reserved == 1) {
            printf(" Found %s Drive %dGB - %s\n",
                (const char *[]){"ATA", "ATAPI"}[s_devices[i].type],         /* Type */
                s_devices[i].size / 1024 / 1024 / 2,               /* Size */
                s_devices[i].model);
    }

    return 0;
}

void IDE::ideWrite(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0C)
      ideWrite(channel, ATA_REG_CONTROL, 0x80 | s_channels[channel].nIEN);
    if (reg < 0x08)
        outb(s_channels[channel].base  + reg - 0x00, data);
    else if (reg < 0x0C)
        outb(s_channels[channel].base  + reg - 0x06, data);
    else if (reg < 0x0E)
        outb(s_channels[channel].ctrlBase  + reg - 0x0A, data);
    else if (reg < 0x16)
        outb(s_channels[channel].bmide + reg - 0x0E, data);
    if (reg > 0x07 && reg < 0x0C)
        ideWrite(channel, ATA_REG_CONTROL, s_channels[channel].nIEN);
}

uint8_t IDE::ideRead(uint8_t channel, uint8_t reg) {
    unsigned char result;
    if (reg > 0x07 && reg < 0x0C)
        ideWrite(channel, ATA_REG_CONTROL, 0x80 | s_channels[channel].nIEN);
    if (reg < 0x08)
        result = inb(s_channels[channel].base + reg - 0x00);
    else if (reg < 0x0C)
        result = inb(s_channels[channel].base  + reg - 0x06);
    else if (reg < 0x0E)
        result = inb(s_channels[channel].ctrlBase  + reg - 0x0A);
    else if (reg < 0x16)
        result = inb(s_channels[channel].bmide + reg - 0x0E);
    if (reg > 0x07 && reg < 0x0C)
        ideWrite(channel, ATA_REG_CONTROL, s_channels[channel].nIEN);
    return result;
}

void IDE::ideReadBuffer(uint8_t channel, uint8_t reg, void* buffer, uint32_t count) {
    if (reg > 0x07 && reg < 0x0C)
      ideWrite(channel, ATA_REG_CONTROL, 0x80 | s_channels[channel].nIEN);
   asm("pushw %es; movw %ds, %ax; movw %ax, %es");
   if (reg < 0x08)
      insl(s_channels[channel].base  + reg - 0x00, (uint32_t*)buffer, count);
   else if (reg < 0x0C)
      insl(s_channels[channel].base  + reg - 0x06, (uint32_t*)buffer, count);
   else if (reg < 0x0E)
      insl(s_channels[channel].ctrlBase  + reg - 0x0A, (uint32_t*)buffer, count);
   else if (reg < 0x16)
      insl(s_channels[channel].bmide + reg - 0x0E, (uint32_t*)buffer, count);
   asm("popw %es;");
   if (reg > 0x07 && reg < 0x0C)
      ideWrite(channel, ATA_REG_CONTROL, s_channels[channel].nIEN);
}

uint8_t IDE::idePolling(uint8_t channel, bool advancedCheck) {
    // Waste 400 nanoseconds
    for (int i = 0; i < 4; i++) ideRead(channel, ATA_REG_ALTSTATUS);

    while (ideRead(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    if (advancedCheck) {
        uint8_t state = ideRead(channel, ATA_REG_STATUS);

        if (state & ATA_SR_ERR) return 2; // Error
        if (state & ATA_SR_DF) return 1; // Device Fault
        if ((state & ATA_SR_DRQ) == 0) return 3;
    }

    return 0;
}

uint8_t IDE::idePrintError(uint8_t drive, uint8_t err) {
    if (err == 0)
      return err;

    printf("IDE:");

    if (err == 1) {
        printf("- Device Fault\n     ");
        err = 19;
    }
    else if (err == 2) {
        unsigned char st = ideRead(s_devices[drive].channel, ATA_REG_ERROR);
        if (st & ATA_ER_AMNF)   {
            printf("- No Address Mark Found\n     ");
            err = 7;
        }
        if (st & ATA_ER_TK0NF)   {
            printf("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_ABRT)   {
            printf("- Command Aborted\n     ");   
            err = 20;
        }
        if (st & ATA_ER_MCR)   {
            printf("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_IDNF)   {
            printf("- ID mark not Found\n     ");   
            err = 21;
        }
        if (st & ATA_ER_MC)   {
            printf("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_UNC)   {
            printf("- Uncorrectable Data Error\n     ");
            err = 22;
        }
        if (st & ATA_ER_BBK)   {
            printf("- Bad Sectors\n     ");    
            err = 13;
        }
    } else  if (err == 3)           {
        printf("- Reads Nothing\n     ");
        err = 23;
    }
    else  if (err == 4)  {
        printf("- Write Protected\n     ");
        err = 8;
    }

    printf("- [%s %s] %s\n",
        (const char *[]){"Primary", "Secondary"}[s_devices[drive].channel], // Use the channel as an index into the array
        (const char *[]){"Master", "Slave"}[s_devices[drive].drive], // Same as above, using the drive
        s_devices[drive].model);

    return err;
}