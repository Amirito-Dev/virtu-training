#include "disk.h"
#include <linux/kvm.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void init_disk(disk_t *disk, int fd, int size){
    disk->seq_idx = 0;
    disk->state = PERIPH_NOT_READY;
    disk->lba = 0;
    memset(disk->buffer, 0, 512);
    disk->byte_already_written = 0;
    disk->fd = fd;
    disk->size = size;
}

void handle_disk_io(vmm_t *vmm){
    struct kvm_run *r = vmm->run;

    bool step_ok = false;

    uint8_t *addr = (uint8_t *)r + r->io.data_offset;
    uint32_t value = *addr;

    if(r->io.port == STATUS_PORT && r->io.direction == KVM_EXIT_IO_IN){
        *addr = 0x40;
        return;
    }

    switch(vmm->disk.seq_idx){
        case 0:
            if(r->io.port == DATA_PORT+2 && value == 1) step_ok = true;
            break;
        case 1: 
            if(r->io.port == DATA_PORT+3){ vmm->disk.lba = value; step_ok = true; }
            break;
        case 2: 
            if(r->io.port == DATA_PORT+4){ vmm->disk.lba |= value << 8; step_ok = true; }
            break;
        case 3: 
            if(r->io.port == DATA_PORT+5){ vmm->disk.lba |= value << 16; step_ok = true; }
            break;
        case 4: 
            if(r->io.port == DATA_PORT+6){ vmm->disk.lba |= (value & 0x0f) << 24; step_ok = true; }
            break;
        case 5: 
            if(r->io.port == STATUS_PORT && value == 0x30 && r->io.direction == KVM_EXIT_IO_OUT){
                step_ok = true;
            }
            break;
        case 6: // recevoir les données : On y reste 256 fois
            if(r->io.port == DATA_PORT && r->io.direction == KVM_EXIT_IO_OUT){
                uint16_t val16 = *(uint16_t *)addr;
                memcpy(&vmm->disk.buffer[vmm->disk.byte_already_written], &val16, 2);
                vmm->disk.byte_already_written += 2;
                step_ok = true;
            }
            break;
    }

    if(step_ok){
        // Si on est dans les étapes de configuration (0 à 5), on avance
        if (vmm->disk.seq_idx < 6) {
            vmm->disk.seq_idx++;
        } 
        // Si on est à l'étape 6, on n'avance plus, on attend d'avoir 512 octets
        else if (vmm->disk.seq_idx == 6 && vmm->disk.byte_already_written == 512) {
            printf("Secteur complet recu, écriture sur le disque...\n");
            
            pwrite(vmm->disk.fd, vmm->disk.buffer, 512, vmm->disk.lba*512);
            
            // Une fois écrit, on reset pour la prochaine écriture
            init_disk(&vmm->disk, vmm->disk.fd, vmm->disk.size);
        }
    } else {
        init_disk(&vmm->disk, vmm->disk.fd, vmm->disk.size);
    }

}