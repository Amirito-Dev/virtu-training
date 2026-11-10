#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "x86.h"
#include "pmio.h"
#include "serial.h"
#include "graphic.h"
#include "disk.h"

void guest_main() {
    idt_init();  // Initialize interrupt subsystem
    sti();       // Enable hardware interrupts

    if(init_serial(DIVIDER) != 0){
        halt();
    }

    write_serial('O');
    write_serial('U');
    write_serial('I');

    init_graphic(5);
    fill_screen(255, 0, 0);

    uint8_t sector_data[SECTOR_SIZE];

    for(int i = 0; i < SECTOR_SIZE; i++){
        sector_data[i] = 12;
    }

    write_sector(0, sector_data);
}
