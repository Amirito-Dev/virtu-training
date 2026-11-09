#include <stdint.h>

#define PIXEL_SIZE_IN_BYTE 3

int init_graphic(int mode);
void fill_screen(uint8_t r, uint8_t g, uint8_t b);
void put_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);