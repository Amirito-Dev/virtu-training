#include <stdint.h>
#include "serial.h"
#include "graphic.h"

static uint32_t *GRAPHICAL_STATUS = (uint32_t*)0x4E700000;
static uint32_t *GRAPHICAL_COMMAND = (uint32_t*)0x4E700100;
static uint32_t *GRAPHICAL_DATA = (uint32_t*)0x4E700200;
static uint32_t FRAMEBUFFER_ADDR = 0;
static int SCREEN_WIDTH = 0;
static int SCREEN_HEIGHT = 0;

int init_graphic(int mode){
    // Bloquer tant qu'on ne reçoit pas 17
    while(*(uint16_t*)GRAPHICAL_STATUS != 17){}

    switch(mode){
        case 1:
            SCREEN_WIDTH = 1600;
            SCREEN_HEIGHT = 900;
        break;
        case 2:
            SCREEN_WIDTH = 1280;
            SCREEN_HEIGHT = 720;
        break;
        case 3:
            SCREEN_WIDTH = 1024;
            SCREEN_HEIGHT = 576;
        break;
        case 4:
            SCREEN_WIDTH = 800;
            SCREEN_HEIGHT = 500;
        break;
        case 5:
            SCREEN_WIDTH = 640;
            SCREEN_HEIGHT = 400;
        break;
    }
    
    *(uint16_t*)GRAPHICAL_COMMAND = 5;

    *(uint16_t*)GRAPHICAL_DATA = mode;

    FRAMEBUFFER_ADDR = *GRAPHICAL_DATA;

    // Si on a pas récupéré l'adresse du frame buffer, alors fail
    if(FRAMEBUFFER_ADDR == 0){
        return 1;
    }

    return 0;
}

void fill_screen(uint8_t r, uint8_t g, uint8_t b){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
        for(int x = 0; x < SCREEN_WIDTH; x++){
            put_pixel(x, y, r, g, b);
        }
    }
}

void put_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b){
    // Convertir l'adresse physique en pointeur
    uint8_t *fb_ptr = (uint8_t*)FRAMEBUFFER_ADDR;
    int offset = (y * SCREEN_WIDTH + x) * PIXEL_SIZE_IN_BYTE;

    fb_ptr[offset] = r;
    fb_ptr[offset+1] = g;
    fb_ptr[offset+2] = b;
}