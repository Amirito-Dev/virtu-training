#include <stdint.h>
#include "types.h"

#define MIN_RAM 512*1024
#define MAX_RAM 4*1024*1024
#define MACHINE_TYPE 0
#define RAM_SLOT 0
#define STACK_SLOT 1
#define RAM_START 32*1024
#define STACK_START 8*1024*1024
#define STACK_SIZE 2*1024*1024

void vmm_init(vmm_t *vmm, uint32_t ram_size, char* guest_path);
void vmm_run(vmm_t *vmm);