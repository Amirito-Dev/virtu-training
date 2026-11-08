#include <stdint.h>

static uint32_t *GRAPHICAL_STATUS = (uint32_t*)0x4E700000;
static uint32_t *GRAPHICAL_COMMAND = (uint32_t*)0x4E700100;
static uint32_t *GRAPHICAL_DATA = (uint32_t*)0x4E700200;
static uint32_t FRAMEBUFFER_ADDR = 0;

void init_graphic(int mode){
    // Bloquer tant qu'on ne reçoit pas 17
    while(*(uint16_t*)GRAPHICAL_STATUS != 17){}

    *(uint16_t*)GRAPHICAL_COMMAND = 5;
    *(uint16_t*)GRAPHICAL_DATA = mode;
    FRAMEBUFFER_ADDR = *GRAPHICAL_DATA = mode;

    // Si on a pas récupérer l'adresse du frame buffer, alors fail
    if(FRAMEBUFFER_ADDR == 0){
        return 1;
    }

    return 0;
}
