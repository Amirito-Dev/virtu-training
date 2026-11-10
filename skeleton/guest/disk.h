#define SECTOR_SIZE    512      // A sector has a size of 512 bytes

#define STATUS_PORT    0x1F7    // Status register on the primary bus (when read from)
#define CMD_PORT       0x1F7    // Command register on the primary bus (when written to)
#define DATA_PORT      0x1F0    // Base port on primary bus

/**
 * Wait for the disk drive to be ready.
 */
void wait_drive();
/**
 * Prepare the disk drive for read/write at the specified sector in 28-bit LBA mode.
 * @param sector the sector to read or write
 */
void pio_prepare(int sector);
/**
 * Read a sector from the first disk.
 * @param sector sector to read (0-indexed)
 * @param dst address to store to read data
 * Based on the assembly code at http://wiki.osdev.org/ATA_read/write_sectors
 */
void read_sector(int sector, void *dst);
/**
 * Write a sector from the first disk.
 * @param sector sector to write (0-indexed)
 * @param src address of the data to be written
 * Based on the assembly code at http://wiki.osdev.org/ATA_read/write_sectors
 */
void write_sector(int sector, void *src);
