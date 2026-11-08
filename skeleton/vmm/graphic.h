#include <linux/kvm.h>
#include "types.h"

#define GRAPHICAL_STATUS 0x4E700000
#define GRAPHICAL_COMMAND 0x4E700100
#define GRAPHICAL_DATA 0x4E700200
#define GRAPHICAL_PHYS_ADDR 10*1024*1024 // après la stack
#define GRAPHIC_SLOT 2
#define BYTES_PER_PIXEL 3
#define FOUR_KB 4096

void init_graphic(graphic_t *graphic);

void handle_graphic_mmio(vmm_t *vmm);