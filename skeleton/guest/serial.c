#include "serial.h"

// Max transmission speed is 115200 bauds; speed_div is the speed divisor -> speed = 115200/speed_div
int init_serial(uint16_t speed_div) {
    outb(SERIAL_PORT + 1, 0x00); // Disable all interrupts
    outb(SERIAL_PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(SERIAL_PORT + 0, speed_div & 0xFF); // Set divisor to speed_div (lo byte)
    outb(SERIAL_PORT + 1, (speed_div & 0xFF00) >> 8); // (hi byte)
    outb(SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(SERIAL_PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(SERIAL_PORT + 0, 0xAE); // Send byte 0xAE and check serial returns the same
    if (inb(SERIAL_PORT + 0) != 0xAE) { return 1; }
    outb(SERIAL_PORT + 4, 0x0F); // Set normal operation mode
    return 0;
}
uint8_t read_serial() {
    while ((inb(SERIAL_PORT + 5) & 1) == 0);
    return inb(SERIAL_PORT);
}

void write_serial(uint8_t val) {
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
    outb(SERIAL_PORT, val);
}