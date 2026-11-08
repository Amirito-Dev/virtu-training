#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "x86.h"
#include "pmio.h"

#define PORT 0x3f8 // First serial interface (aka COM1)
#define DIVIDER 10000

// Max transmission speed is 115200 bauds; speed_div is the speed divisor -> speed = 115200/speed_div
static int init_serial(uint16_t speed_div) {
    outb(PORT + 1, 0x00); // Disable all interrupts
    outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, speed_div & 0xFF); // Set divisor to speed_div (lo byte)
    outb(PORT + 1, (speed_div & 0xFF00) >> 8); // (hi byte)
    outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + 0, 0xAE); // Send byte 0xAE and check serial returns the same
    if (inb(PORT + 0) != 0xAE) { return 1; }
    outb(PORT + 4, 0x0F); // Set normal operation mode
    return 0;
}
uint8_t read_serial() {
    while ((inb(PORT + 5) & 1) == 0);
    return inb(PORT);
}

void write_serial(uint8_t val) {
    while ((inb(PORT + 5) & 0x20) == 0);
    outb(PORT, val);
}

void guest_main() {
    idt_init();  // Initialize interrupt subsystem
    sti();       // Enable hardware interrupts

    if(init_serial(DIVIDER) != 0){
        halt();
    }

    write_serial('O');
    write_serial('U');
    write_serial('I');
}
