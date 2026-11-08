#include "types.h"

#define SERIAL_PORT 0x3f8 // First serial interface (aka COM1)

void init_serial(serial_t *serial);
void handle_serial_pmio(vmm_t *vmm);