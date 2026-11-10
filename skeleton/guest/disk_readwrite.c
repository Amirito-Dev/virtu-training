#include <stdint.h>
#include "pmio.h"
#include "disk.h"

/**
 * Wait for the disk drive to be ready.
 */
void wait_drive() {
    while ((inb(STATUS_PORT) & 0xC0) != 0x40);
}

/**
 * Prepare the disk drive for read/write at the specified sector in 28-bit LBA mode.
 * @param sector the sector to read or write
 */

 void pio_prepare(int sector) {
    wait_drive();
    outb(DATA_PORT+2, 1);  // sector count
    outb(DATA_PORT+3, sector & 0xff);  // send bits 0-7 of LBA
    outb(DATA_PORT+4, (sector >> 8) & 0xff);  // send bits 8-15 of LBA
    outb(DATA_PORT+5, (sector >> 16) & 0xff);  // send bits 16-23 of LBA
    outb(DATA_PORT+6, ((sector >> 24) & 0x0f) | 0xe0);  // send bits 24-27 of LBA + set LBA mode; 0xe0 = 11100000b;
}

/**
 * Read a sector from the first disk.
 * @param sector sector to read (0-indexed)
 * @param dst address to store to read data
 * Based on the assembly code at http://wiki.osdev.org/ATA_read/write_sectors
 */
void read_sector(int sector, void *dst) {
    pio_prepare(sector);
    outb(CMD_PORT, 0x20);  // command port: read with retry
    wait_drive();

    uint16_t *data = (uint16_t *)dst;
    for (int i = 0; i < SECTOR_SIZE/2; i++) {
        *data = inw(DATA_PORT);
        data++;
    }
}

/**
 * Write a sector from the first disk.
 * @param sector sector to write (0-indexed)
 * @param src address of the data to be written
 * Based on the assembly code at http://wiki.osdev.org/ATA_read/write_sectors
 */
void write_sector(int sector, void *src) {
    pio_prepare(sector);
    outb(CMD_PORT, 0x30);  // command port: write with retry
    wait_drive();

    uint16_t *data = (uint16_t *)src;
    for (int i = 0; i < SECTOR_SIZE/2; i++) {
        outw(DATA_PORT, *data);
        data++;
    }
}
