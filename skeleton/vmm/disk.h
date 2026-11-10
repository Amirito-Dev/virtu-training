#include "types.h"

#define STATUS_PORT    0x1F7    // Status register on the primary bus (when read from)
#define CMD_PORT       0x1F7    // Command register on the primary bus (when written to)
#define DATA_PORT      0x1F0    // Base port on primary bus

void init_disk(disk_t *disk, int fd, int size);
void handle_disk_io(vmm_t *vmm);