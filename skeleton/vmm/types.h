#pragma once

#include <stdint.h>
#include <linux/kvm.h>

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
    int kvm_fd;
    int vm_fd;
    int vcpu_fd;
    uint32_t ram_size;
    struct kvm_run *run;
    serial_t serial;
} vmm_t;

