#include "keyboard.h"
#include <linux/kvm.h>
#include <stdio.h>

void init_keyboard(keyboard_t *keyboard){
    keyboard->currentKey = 0;
}

void handle_keyboard_io(vmm_t *vmm){
    struct kvm_run *r = vmm->run;

    if(r->io.port != KEYBOARD_PORT){
        return;
    }

    if(r->io.direction == KVM_EXIT_IO_IN && r->io.size == 4){
        uint8_t *addr = (uint8_t *)r + r->io.data_offset;
        
        *(uint32_t *)addr = vmm->keyboard.currentKey;
        
        if (vmm->keyboard.currentKey != 0) {
            vmm->keyboard.currentKey = 0;
        }
    }

}