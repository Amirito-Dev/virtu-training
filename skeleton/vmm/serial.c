#include "serial.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

void init_serial(serial_t *serial){
    serial->divider = 0;
    serial->seq_idx = 0;
    serial->state = PERIPH_NOT_READY;
    serial->time_to_emulate = 0;
}

void handle_serial_pmio(vmm_t *vmm){
    // Si le port n'appartient pas au port série COM1 (0x3F8 à 0x3FD)
    if (vmm->run->io.port < 0x3F8 || vmm->run->io.port > 0x3FD) {
        return;
    }
    
    //printf("SERIAL PMIO\n");
    uint8_t *addr = (uint8_t *)vmm->run + vmm->run->io.data_offset;
    uint32_t value;
    value = *(uint8_t *)addr;

    // Si le guest demande si le port est prêt
    if(vmm->run->io.direction == KVM_EXIT_IO_IN && vmm->run->io.port == SERIAL_PORT+5){
        *addr = 0x20;
        return;
    }


    if(vmm->serial.state == PERIPH_READY){
        usleep(vmm->serial.time_to_emulate);
        putchar(value);
        return;
    }

    bool step_ok = false;

    if(vmm->run->io.direction == KVM_EXIT_IO_OUT){
        switch(vmm->serial.seq_idx){
            case 0:
            if(vmm->run->io.port == SERIAL_PORT+1 && value == 0x00){
                step_ok = true;
            }
            break;
            case 1:
            if(vmm->run->io.port == SERIAL_PORT+3 && value == 0x80){
                step_ok = true;
            }
            break;
            case 2:
            if(vmm->run->io.port == SERIAL_PORT){
                step_ok = true;
                vmm->serial.divider = value;
            }
            break;
            case 3:
            if(vmm->run->io.port == SERIAL_PORT+1){
                step_ok = true;
                vmm->serial.divider |= value << 8;
            }
            break;
            case 4:
            if(vmm->run->io.port == SERIAL_PORT+3 && value == 0x03){
                step_ok = true;
            }
            break;
            case 5:
            if(vmm->run->io.port == SERIAL_PORT+2 && value == 0xC7){
                step_ok = true;
            }
            break;
            case 6:
            if(vmm->run->io.port == SERIAL_PORT+4 && value == 0x0B){
                step_ok = true;
            }
            break;
            case 7:
            if(vmm->run->io.port == SERIAL_PORT+4 && value == 0x1E){
                step_ok = true;
            }
            break;
            case 8:
            if(vmm->run->io.port == SERIAL_PORT && value == 0xAE){
                step_ok = true;
            }
            break;
            case 10:
            if(vmm->run->io.port == SERIAL_PORT+4 && value == 0x0F){
                step_ok = true;
            }
            break;
        }
        //printf("PMIO guest write: size=%d port=0x%x value=0x%x\n", vmm->run->io.size, vmm->run->io.port, value);
    }else if(vmm->run->io.direction == KVM_EXIT_IO_IN){
        if(vmm->serial.seq_idx == 9){
            step_ok = true;
            *addr = 0xAE;
            vmm->serial.state = PERIPH_READY;
        }
        //printf("PMIO guest read: port=0x%x size=%d [value injected by VMM=%d]\n", vmm->run->io.port, vmm->run->io.size, 42);
    }


    if(step_ok){
        vmm->serial.seq_idx += 1;
        if(vmm->serial.seq_idx == 10){
            vmm->serial.time_to_emulate = vmm->serial.divider * 10;
            printf("Periphérique serial init avec succès | Diviseur : %d | temps à émuler r: %d us\n", vmm->serial.divider, vmm->serial.time_to_emulate);
        }
    }else{
        init_serial(&vmm->serial);
    }


}