#include "graphic.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <err.h>
#include <sys/ioctl.h>

void init_graphic(graphic_t *graphic){
    graphic->framebuffer_addr = 0;
    graphic->seq_idx = 0;
    graphic->state = PERIPH_NOT_READY;  
    graphic->mode = 0;
    graphic->res = resolutions[0];
    sem_init(&graphic->init_sem, 0, 0);
}

void handle_graphic_mmio(vmm_t *vmm){

    if(vmm->graphic.state == PERIPH_READY){
        return;
    }

    bool step_ok = false;

    if(vmm->run->mmio.is_write){
        int bytes_written = vmm->run->mmio.len;
        uint32_t value;
        value = *((uint16_t *)vmm->run->mmio.data);

        if(vmm->graphic.seq_idx == 1){
            step_ok = true;
        }else if(vmm->graphic.seq_idx == 2 && value >= 1 && value <= 5){
            //printf("MODE : %d\n", value);
            vmm->graphic.mode = value;
            vmm->graphic.res = resolutions[value-1];
            printf("width : %d height : %d\n", vmm->graphic.res.width, vmm->graphic.res.height);
            step_ok = true;
        }

        //printf("MMIO guest write: addr=0x%llx value=0x%x len=%d\n", vmm->run->mmio.phys_addr, value, bytes_written);

    }else if(!vmm->run->mmio.is_write){
        if(vmm->graphic.seq_idx == 0){
            int bytes_read = vmm->run->mmio.len;
            uint16_t *addr = (uint16_t *)vmm->run->mmio.data;
            *addr = 17;
            step_ok = true;
        }else if(vmm->graphic.seq_idx == 3){
            uint32_t *addr = (uint32_t *)vmm->run->mmio.data;
            *addr = GRAPHICAL_PHYS_ADDR;
            
            // Allouer le framebuffer en mémoire
            int framebuffer_size = vmm->graphic.res.width * vmm->graphic.res.height * BYTES_PER_PIXEL;

            int rest = FOUR_KB - (framebuffer_size % FOUR_KB);
            framebuffer_size += rest;

            //printf("Framebuffer size : %d | reste : %d\n", framebuffer_size, rest);

            uint8_t *framebuffer_ptr = mmap(NULL, framebuffer_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
            if (framebuffer_ptr == MAP_FAILED) err(1, "Error: failed allocating guest memory: ");

            struct kvm_userspace_memory_region ramreg = {
            .slot = GRAPHIC_SLOT,
            .guest_phys_addr = GRAPHICAL_PHYS_ADDR, // MUST be aligned to a page boundary (4 KB)
            .memory_size = framebuffer_size, // MUST be a multiple of a page size (4 KB)
            .userspace_addr = (uint64_t)framebuffer_ptr,
            .flags = 0
            };
            if (ioctl(vmm->vm_fd, KVM_SET_USER_MEMORY_REGION, &ramreg) < 0) err(1, "Error: KVM_SET_USER_MEMORY_REGION: ");

            vmm->graphic.graphic_ptr = framebuffer_ptr;
            step_ok = true;
        }
   }


    if(step_ok){
        //printf("STEP %d OK\n", vmm->graphic.seq_idx);
        vmm->graphic.seq_idx += 1;
        if(vmm->graphic.seq_idx == 4){
            printf("Périphérique graphique initialisé.\n");
            vmm->graphic.state = PERIPH_READY;
            sem_post(&vmm->graphic.init_sem);
        }
    }else{
        //printf("STEP %d NOT OK\n", vmm->graphic.seq_idx);
        init_graphic(&vmm->graphic);
    }

}