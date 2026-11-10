#include "keyboard.h"
#include "pmio.h"
#include "serial.h"

void read_key(uint32_t key){
    while(ind(0x60) != key){}
    write_serial('K');
    write_serial('E');
    write_serial('Y');
    write_serial('\n');
}