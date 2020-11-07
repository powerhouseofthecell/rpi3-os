#include "kernel/kernel.hh"
#include "kernel/uart.hh"
#include "kernel/mmu.hh"
#include "kernel/lfb.hh"
#include "kernel/vmiter.hh"
#include "common/console.hh"

#include "common/types.hh"
#include "common/stdlib.hh"

#define KERNEL_UART0_DR        ((volatile unsigned int*)0xFFFFFFFFFFE00000)
#define KERNEL_UART0_FR        ((volatile unsigned int*)0xFFFFFFFFFFE00018)

extern volatile unsigned char _data;

// marks what we'll pretend is the beginning of physical memory
extern volatile unsigned char _end;

// Memory state
//    Information about physical page with address `pa` is stored in
//    `pages[pa / PAGESIZE]`. In the handout code, each `pages` entry
//    holds an `refcount` member, which is 0 for free pages.
//    You can change this as you see fit.

pageinfo pages[NPAGES];

void dissect_vaddr(Console console, unsigned long vaddr) {
    console.printf("dissecting           : %p\n", vaddr);
    console.printf("rg sel: bits [39, 64): 0x%x\n", vaddr >> 39);
    console.printf("l1 idx: bits [30, 39): 0x%x\n", pageindex(vaddr, 2));
    console.printf("l2 idx: bits [21, 30): 0x%x\n", pageindex(vaddr, 1));
    console.printf("l3 idx: bits [12, 21): 0x%x\n", pageindex(vaddr, 0));
    console.printf("pa idx: bits [00, 12): 0x%x\n", pageoffset(vaddr, 0));
}

extern "C" void kernel_main() {
    // set up serial console and linear frame buffer
    uart_init();
    mmu_init();
    lfb_init();

    // initialize the console now that the lfb has been set
    Console console((uint64_t) lfb);
    
    console.printf("lfb: %p\n", lfb);
    console.printf("Current Level: %i\n", getCurrentEL());
    console.printf("_end: %p, _data: %p\n", &_end, &_data);

    // identity map the kernel, skipping 0x0 (nullptr)
    for (vmiter it((pagetable*) &_end, 0x1000); it.va() < MEMSIZE_PHYSICAL; it += PAGESIZE) {
        if (it.va() < 0x80000 || it.va() >= (uintptr_t) &_data) {
            it.map(it.va(), PTE_PAGE | PTE_A | PTE_PWU | (3<<8));
        } else {
            it.map(it.va(), PTE_PAGE | PTE_A | PTE_PRU | (3<<8));
        } 
    }
    
    // echo everything back
    while (true) {
        int c = toupper(uart_getc());
        uart_putc(c);
    }
}


#define IOPHYSMEM       0x000A0000
#define EXTPHYSMEM      0x00100000

bool reserved_physical_address(uintptr_t pa) {
    return pa < PAGESIZE;// || (pa >= IOPHYSMEM && pa < EXTPHYSMEM);
}

// allocatable_physical_address(pa)
//    Returns true iff `pa` is an allocatable physical address, i.e.,
//    not reserved or holding kernel data.

bool allocatable_physical_address(uintptr_t pa) {
    return !reserved_physical_address(pa)
        && (pa < KERNEL_START_ADDR
            || pa >= round_up((uintptr_t) &_end, PAGESIZE))
        && (pa < KERNEL_STACK_TOP - PAGESIZE
            || pa >= KERNEL_STACK_TOP)
        && pa < MEMSIZE_PHYSICAL;
}

void* kalloc_page() {
    uart_puts("Kalloc'd page\n");
    for (uintptr_t pa = 0; pa != MEMSIZE_PHYSICAL; pa += PAGESIZE) {
        if (allocatable_physical_address(pa)
            && !pages[pa / PAGESIZE].used()) {
            ++pages[pa / PAGESIZE].refcount;
            memset((void*) pa, 0xCC, PAGESIZE);
            return (void*) pa;
        }
    }
    return nullptr;
}