#include <stdio.h>
#include <stdlib.h>
#include "vmm.h"
#include <pthread.h>
#include "gfx.h"

void* vmm_run_thread(void *arg){
    vmm_t *vmm = (vmm_t*)arg;
    vmm_run(vmm);
    
}

void render_framebuffer(gfx_context_t *context, vmm_t *vmm){
    int offset = 0;

    for(int y = 0; y < vmm->graphic.res.height; y++){
        for(int x = 0; x < vmm->graphic.res.width; x++){
            uint8_t r = vmm->graphic.graphic_ptr[offset++];
            uint8_t g = vmm->graphic.graphic_ptr[offset++];
            uint8_t b = vmm->graphic.graphic_ptr[offset++];

            gfx_background_putpixel(context, x, y, GFX_RGB(r, g, b));
        }
    }
}

int main(int argc, char* argv[]){
    printf("argc : %d\n", argc);
    setbuf(stdout, NULL);
    
    if(argc != 3){
        printf("Usage : [ram size] [guest path]\n");
        exit(EXIT_FAILURE);
    }
    uint32_t ram_size = atoi(argv[1]);
    char* guest_path = argv[2];

    vmm_t vmm;

    vmm_init(&vmm, ram_size, guest_path);

    pthread_t vmm_thread;

    pthread_create(&vmm_thread, NULL, vmm_run_thread, &vmm);
    
    sem_wait(&vmm.graphic.init_sem);

    gfx_context_t *ctxt = gfx_create("Basic Example", vmm.graphic.res.width, vmm.graphic.res.height);
    if (!ctxt) {
        fprintf(stderr, "Graphics initialization failed!\n");
        return EXIT_FAILURE;
    }

    while (gfx_keypressed() != SDLK_ESCAPE) {
        render_framebuffer(ctxt, &vmm);
        gfx_background_update(ctxt);
        gfx_present(ctxt);
    }


    gfx_destroy(ctxt);
    pthread_join(vmm_thread, NULL);

    return 0;
}