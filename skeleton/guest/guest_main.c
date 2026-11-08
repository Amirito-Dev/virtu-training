#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "x86.h"
#include "pmio.h"
#include "serial.h"
#include "graphic.h"

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
}
