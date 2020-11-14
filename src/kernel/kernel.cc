#include "kernel/kernel.hh"
#include "kernel/uart.hh"
#include "kernel/mmu.hh"
#include "kernel/lfb.hh"
#include "kernel/vmiter.hh"
#include "kernel/delays.hh"
#include "kernel/timer.hh"

#include "common/types.hh"
#include "common/stdlib.hh"

#define KERNEL_UART0_DR        ((volatile unsigned int*)0xFFFFFFFFFFE00000)
#define KERNEL_UART0_FR        ((volatile unsigned int*)0xFFFFFFFFFFE00018)

extern volatile unsigned char _data;
extern volatile unsigned char _end;

// a pointer to the first kernel pagetable
pagetable* kernel_pagetable = (pagetable*) &_end;

// Memory state
//    Information about physical page with address `pa` is stored in
//    `pages[pa / PAGESIZE]`. In the handout code, each `pages` entry
//    holds an `refcount` member, which is 0 for free pages.
//    You can change this as you see fit.

pageinfo pages[NPAGES];

// handle interrupts
extern "C" void irq_handler() {
    // increment our "clock's" ticks
    // reset after (hr * 60 min/hr * 60 sec/min * HZ irq/sec) irqs
    ticks = (ticks + 1) % (1 * 60 * 60 * HZ);

    // reset the timer
    // TODO: #define the magic numbers here
    *((uint32_t*) 0x40000038) = (uint32_t) (LOCAL_TIMER_RELOAD | (1<<30));

    // restore the cursor position after this printing
    uint32_t old_x = char_xpos;
    uint32_t old_y = char_ypos;
    puts(fbInfo.width / font->width - 12, 0, "Tick: ", BLACK, WHITE);
    puts(fbInfo.width / font->width - 6, 0, itoa(ticks, 10), BLACK, WHITE);
    char_xpos = old_x;
    char_ypos = old_y;
}

// the main initialization function for our kernel
extern "C" void kernel_main() {
    // set up serial console
    uart_init();

    // set up linear frame buffer
    lfb_init();

    // initialize memory and the memory management unit
    mmu_init();

    // initialize interrupts (timer)
    init_interrupts();

    printf("lfb: %p\n", fbInfo.addr);
    printf("Current Level: %i\n", getCurrentEL());
    printf("_end: %p, _data: %p\n", &_end, &_data);

    // loop forever
    while (true) {
        //puts("hi");
    };
}

// prints a dissected physical address (dissected by pagetable entry index bits)
void dissect_vaddr(unsigned long vaddr) {
    printf("dissecting           : %p\n", vaddr);
    printf("rg sel: bits [39, 64): 0x%x\n", vaddr >> 39);
    printf("l1 idx: bits [30, 39): 0x%x\n", pageindex(vaddr, 2));
    printf("l2 idx: bits [21, 30): 0x%x\n", pageindex(vaddr, 1));
    printf("l3 idx: bits [12, 21): 0x%x\n", pageindex(vaddr, 0));
    printf("pa idx: bits [00, 12): 0x%x\n", pageoffset(vaddr, 0));
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