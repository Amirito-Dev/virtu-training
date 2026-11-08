#include "vmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/kvm.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <err.h>
#include <stdbool.h>
#include "serial.h"

void vmm_init(vmm_t *vmm, uint32_t ram_size, char* guest_path){
    if (ram_size < 512 || ram_size > (4*1024*1024)) err(1, "la RAM doit être entre 512KB et 4MO");

    printf("vmm RAM amount : %d bytes\n", ram_size);
    printf("Guest to load : %s\n", guest_path);

    // Connexion à l'API KVM
    int kvmfd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
    if (kvmfd < 0) err(1, "Failed opening kvm device: ");
    vmm->kvm_fd = kvmfd;

    int version = ioctl(kvmfd, KVM_GET_API_VERSION, NULL);
    if (version < 0) err(1, "KVM_GET_API_VERSION failed: ");
    if (version != KVM_API_VERSION) errx(1, "Error: KVM_GET_API_VERSION %d, expected %d!", version, KVM_API_VERSION);

    // Création de la vm
    int vmfd = ioctl(kvmfd, KVM_CREATE_VM, MACHINE_TYPE);
    if (vmfd < 0) err(1, "Error: KVM_CREATE_VM failed: ");
    vmm->vm_fd = vmfd;

    // Allocation de l'espace RAM pour le vmm
    uint8_t *ram_ptr = mmap(NULL, ram_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (ram_ptr == MAP_FAILED) err(1, "Error: failed allocating guest memory: ");

    struct kvm_userspace_memory_region ramreg = {
    .slot = RAM_SLOT,
    .guest_phys_addr = RAM_START, // MUST be aligned to a page boundary (4 KB)
    .memory_size = ram_size, // MUST be a multiple of a page size (4 KB)
    .userspace_addr = (uint64_t)ram_ptr,
    .flags = 0
    };
    if (ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &ramreg) < 0) err(1, "Error: KVM_SET_USER_MEMORY_REGION: ");


    // Charger l'OS Guest dans la RAM
    struct stat sb;
    if (lstat(guest_path, &sb) == -1) {
        perror("lstat");
        err(1, "Erreur lors de la lecture du fichier fourni pour l'OS Guest");
    }
    int guest_fd = open(guest_path, O_RDONLY);
    if(sb.st_size > ram_size){
        err(1, "OS trop gros");
    }

    read(guest_fd, ram_ptr, sb.st_size);
    close(guest_fd);

    // Allocation de l'espace STACK pour le vmm
    uint8_t *stack_ptr = mmap(NULL, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (stack_ptr == MAP_FAILED) err(1, "Error: failed allocating guest memory: ");

    struct kvm_userspace_memory_region stackreg = {
    .slot = STACK_SLOT,
    .guest_phys_addr = STACK_START, // MUST be aligned to a page boundary (4 KB)
    .memory_size = STACK_SIZE, // MUST be a multiple of a page size (4 KB)
    .userspace_addr = (uint64_t)stack_ptr,
    .flags = 0
    };
    if (ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &stackreg) < 0) err(1, "Error: KVM_SET_USER_MEMORY_REGION: ");


    // Création du vCPU
    int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
    if (vcpufd < 0) err(1, "Error: KVM_CREATE_VCPU: ");
    vmm->vcpu_fd = vcpufd;
    int vcpu_mmap_sz = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
    if (vcpu_mmap_sz < 0) err(1, "Error: KVM_GET_VCPU_MMAP_SIZE: ");
    if (vcpu_mmap_sz < sizeof(struct kvm_run)) err(1, "Error: KVM_GET_VCPU_MMAP_SIZE unexpectedly small: ");

    struct kvm_run *run = mmap(NULL, vcpu_mmap_sz, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);
    if (run == MAP_FAILED) {
        err(1, "Error: failed allocating guest memory with mmap");
    }
    vmm->run = run;

    // Initialisation des segments registers du vCPU
    struct kvm_sregs sregs;
    if (ioctl(vcpufd, KVM_GET_SREGS, &sregs) < 0) err(1, "Error: KVM_GET_SREGS: ");
    sregs.cs.base = 0; sregs.cs.selector = 0;
    sregs.ds.base = 0; sregs.ds.selector = 0;
    sregs.es.base = 0; sregs.es.selector = 0;
    sregs.ss.base = 0; sregs.ss.selector = 0;
    if (ioctl(vcpufd, KVM_SET_SREGS, &sregs) < 0) err(1, "Error: KVM_SET_SREGS: ");

    // Initialisation des registres du vCPU
    struct kvm_regs regs;
    memset(&regs, 0, sizeof(regs));
    // rip = adresse de la première instruction que la vm va executer
    // Donc le début de la ram car OS Guest mis dans la RAM et on veut l'éxecuter
    regs.rip = RAM_START;
    // rsp = register stack pointer, donc tout en haut de la stack
    // debut 8mo + taille 2mo = 10mo
    regs.rsp = STACK_START + STACK_SIZE;
    regs.rflags = 0x2;
    if (ioctl(vcpufd, KVM_SET_REGS, &regs) < 0) err(1, "Error: KVM_SET_REGS:");

    init_serial(&vmm->serial);
}

void vmm_run(vmm_t *vmm){
    bool done = false;
    while (!done) {
        // Runs the vCPU until encoutering a VM_EXIT (blocking call)
        if (ioctl(vmm->vcpu_fd, KVM_RUN, NULL) < 0) err(1, "Error: KVM_RUN: ");
        switch (vmm->run->exit_reason) { // See struct kvm_run in "(6) Create a vCPU"
            case KVM_EXIT_IO: // Encountered a PMIO VMexit
                //printf("PMIO VMEXIT\n");
                handle_serial_pmio(vmm);
            break; // This VMexit should be handled here...
            case KVM_EXIT_MMIO: // Encountered a MMIO VMexit
            break; // This VMexit should be handled here...
            case KVM_EXIT_HLT: // "halt" CPU instruction
                printf("Encounterd HLT instruction\n");
                done = true;
            break;
            case KVM_EXIT_SHUTDOWN: // CPU typically encountered a triple fault
            case KVM_EXIT_FAIL_ENTRY:
            case KVM_EXIT_INTERNAL_ERROR:
            default:
                fprintf(stderr, "KVM error");
                done = true;
            break;
        }
    }
}