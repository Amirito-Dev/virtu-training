#pragma once

#include <stdint.h>
#include <linux/kvm.h>
#include <semaphore.h>

typedef enum{
    PERIPH_NOT_READY,
    PERIPH_READY
} periph_state_enum;

typedef struct{
    uint32_t divider;
    periph_state_enum state;
    uint8_t seq_idx;
    int time_to_emulate;
} serial_t;

typedef struct{
    int width;
    int height;
} resolution_t;

static const resolution_t resolutions[] = {
    {1600, 900},
    {1280, 720},
    {1024, 576},
    {800, 500},
    {640, 400},
};

typedef struct{
    periph_state_enum state;
    uint32_t *framebuffer_addr;
    uint8_t seq_idx;
    uint8_t mode;
    resolution_t res;
    uint8_t *graphic_ptr;
    sem_t init_sem; 
} graphic_t;

typedef struct{
    periph_state_enum state;
    uint8_t seq_idx;
    uint32_t lba;
    uint8_t buffer[512];
    int byte_already_written;
    int fd;
    int size;
} disk_t;

typedef struct{
    int kvm_fd;
    int vm_fd;
    int vcpu_fd;
    uint32_t ram_size;
    struct kvm_run *run;
    serial_t serial;
    graphic_t graphic;
    disk_t disk;
} vmm_t;

