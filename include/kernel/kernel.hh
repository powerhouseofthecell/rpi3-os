#ifndef RPI3OS_KERNEL_HH
#define RPI3OS_KERNEL_HH
#include "common/types.hh"
#include "kernel/pagetable.hh"

// Kernel start address
#define KERNEL_START_ADDR       0x40000

// Top of the kernel stack
#define KERNEL_STACK_TOP        0x80000

// First application-accessible address
#define PROC_START_ADDR         0x100000

// Physical memory size
#define MEMSIZE_PHYSICAL        0x200000

// Number of physical pages
#define NPAGES                  (MEMSIZE_PHYSICAL / PAGESIZE)

// Virtual memory size
#define MEMSIZE_VIRTUAL         0x300000

struct pageinfo {
    uint8_t refcount;

    bool used() const {
        return this->refcount != 0;
    }
};
extern pageinfo pages[NPAGES];

// allocatable_physical_address(pa)
//    Returns true iff `pa` is an allocatable physical address.
bool allocatable_physical_address(uintptr_t pa);

// allocates a kernel page, returns nullptr on failure
void* kalloc_page();

#endif