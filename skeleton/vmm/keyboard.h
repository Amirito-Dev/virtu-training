#include "types.h"

#define KEYBOARD_PORT 0x60

void init_keyboard(keyboard_t *keyboard);

void handle_keyboard_io(vmm_t *vmm);