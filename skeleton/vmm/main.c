#include <stdio.h>
#include <stdlib.h>
#include "vmm.h"

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
    vmm_run(&vmm);

    return 0;
}