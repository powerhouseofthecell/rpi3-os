#include "kernel/kernel.hh"
#include "kernel/uart.hh"
#include "kernel/mmu.hh"
#include "kernel/lfb.hh"
#include "common/console.hh"

#include "common/types.hh"
#include "common/stdlib.hh"

#define KERNEL_UART0_DR        ((volatile unsigned int*)0xFFFFFFFFFFE00000)
#define KERNEL_UART0_FR        ((volatile unsigned int*)0xFFFFFFFFFFE00018)

extern volatile unsigned char _data;
extern volatile unsigned char _end;

// Memory state
//    Information about physical page with address `pa` is stored in
//    `pages[pa / PAGESIZE]`. In the handout code, each `pages` entry
//    holds an `refcount` member, which is 0 for free pages.
//    You can change this as you see fit.

pageinfo pages[NPAGES];

void dissect_vaddr(Console console, unsigned long vaddr) {
    console.printf("dissecting           : %p\n", vaddr);
    console.printf("rg sel: bits [37, 64): 0x%x\n", vaddr >> 37);
    console.printf("l1 idx: bits [30, 37): 0x%x\n", (vaddr >> 30) & 127);
    console.printf("l2 idx: bits [21, 30): 0x%x\n", (vaddr >> 21) & 511);
    console.printf("l3 idx: bits [12, 21): 0x%x\n", (vaddr >> 12) & 511);
    console.printf("pa idx: bits [00, 12): 0x%x\n", vaddr & 4095);
}

extern "C" void kernel_main() {
    // set up serial console and linear frame buffer
    uart_init();
    mmu_init();
    lfb_init();

    Console console((uint64_t) lfb);
    
    console.printf("lfb: %p\n", lfb);
    console.printf("Current Level: %i\n", getCurrentEL());
    console.printf("_end: %p, _data: %p\n", &_end, &_data);
    int i = 0;
    console.printf("stack addr: %p\n", &i);

    unsigned long* ttbr0_el1;
    asm volatile ("mrs %0, ttbr0_el1" : "=r" (ttbr0_el1));
    console.printf("l1 addr: %x\n", ttbr0_el1);

    unsigned long l1_entry = *ttbr0_el1;
    console.printf("l1 entry: 0x%x\n", l1_entry);

    unsigned long l2_addr = (unsigned long) ((l1_entry >> 12) << 12);
    console.printf("l2 addr: %p\n", l2_addr);

    unsigned long l2_entry = *((unsigned long*) l2_addr);
    console.printf("l2 entry: 0x%x\n", l2_entry);

    unsigned long l3_addr = (unsigned long) ((l2_entry >> 12) << 12);
    console.printf("l3 addr: %p\n", l3_addr);

    int idx = 0;
    unsigned long l3_entry = *((unsigned long*) (l3_addr + idx));
    console.printf("l3 entry @ %i: 0x%x\n", idx, l3_entry);

    dissect_vaddr(console, (unsigned long) &i);
    
    // echo everything back
    while (true) {
        int c = toupper(uart_getc());
        uart_putc(c);
    }
}


#define IOPHYSMEM       0x000A0000
#define EXTPHYSMEM      0x00100000

bool reserved_physical_address(uintptr_t pa) {
    return pa < PAGESIZE || (pa >= IOPHYSMEM && pa < EXTPHYSMEM);
}

// allocatable_physical_address(pa)
//    Returns true iff `pa` is an allocatable physical address, i.e.,
//    not reserved or holding kernel data.

bool allocatable_physical_address(uintptr_t pa) {
    return !reserved_physical_address(pa)
        && (pa < KERNEL_START_ADDR
            || pa >= round_up((uintptr_t) _end, PAGESIZE))
        && (pa < KERNEL_STACK_TOP - PAGESIZE
            || pa >= KERNEL_STACK_TOP)
        && pa < MEMSIZE_PHYSICAL;
}

void* kalloc(size_t sz) {
    if (sz > PAGESIZE) {
        return nullptr;
    }

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