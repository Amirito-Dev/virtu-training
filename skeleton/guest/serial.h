#include <stdint.h>
#include "pmio.h"

#define SERIAL_PORT 0x3f8 // First serial interface (aka COM1)
#define DIVIDER 10000

// Max transmission speed is 115200 bauds; speed_div is the speed divisor -> speed = 115200/speed_div
int init_serial(uint16_t speed_div);

uint8_t read_serial();

void write_serial(uint8_t val);