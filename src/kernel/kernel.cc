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

void dissect_vaddr(unsigned long vaddr) {
    Console console((uint64_t) lfb);

    console.printf("dissecting           : %p\n", vaddr);
    console.printf("rg sel: bits [39, 64): 0x%x\n", vaddr >> 39);
    console.printf("l1 idx: bits [30, 39): 0x%x\n", pageindex(vaddr, 2));
    console.printf("l2 idx: bits [21, 30): 0x%x\n", pageindex(vaddr, 1));
    console.printf("l3 idx: bits [12, 21): 0x%x\n", pageindex(vaddr, 0));
    console.printf("pa idx: bits [00, 12): 0x%x\n", pageoffset(vaddr, 0));
}

extern "C" void _enable_interrupts();

#define PERIPHERAL_BASE     ((uint64_t)0x3F000000)
#define IRQ_BASIC_PENDING	(PERIPHERAL_BASE+0x0000B200)
#define IRQ_PENDING_1		(PERIPHERAL_BASE+0x0000B204)
#define IRQ_PENDING_2		(PERIPHERAL_BASE+0x0000B208)
#define FIQ_CONTROL		    (PERIPHERAL_BASE+0x0000B20C)
#define ENABLE_IRQS_1		(PERIPHERAL_BASE+0x0000B210)
#define ENABLE_IRQS_2		(PERIPHERAL_BASE+0x0000B214)
#define ENABLE_BASIC_IRQS	(PERIPHERAL_BASE+0x0000B218)
#define DISABLE_IRQS_1		(PERIPHERAL_BASE+0x0000B21C)
#define DISABLE_IRQS_2		(PERIPHERAL_BASE+0x0000B220)
#define DISABLE_BASIC_IRQS	(PERIPHERAL_BASE+0x0000B224)

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)

#define TIMER_CS        (PERIPHERAL_BASE+0x00003000)
#define TIMER_CLO       (PERIPHERAL_BASE+0x00003004)
#define TIMER_CHI       (PERIPHERAL_BASE+0x00003008)
#define TIMER_C0        (PERIPHERAL_BASE+0x0000300C)
#define TIMER_C1        (PERIPHERAL_BASE+0x00003010)
#define TIMER_C2        (PERIPHERAL_BASE+0x00003014)
#define TIMER_C3        (PERIPHERAL_BASE+0x00003018)

#define TIMER_CS_M0	(1 << 0)
#define TIMER_CS_M1	(1 << 1)
#define TIMER_CS_M2	(1 << 2)
#define TIMER_CS_M3	(1 << 3)

const uint32_t interval = 200000;
uint32_t timerVal = 0;

extern "C" void _put32(uint64_t addr, uint32_t val);
extern "C" uint32_t _get32(uint64_t addr);

void init_interrupts() {
    // initialize the timer
    timerVal = _get32(TIMER_CLO);
    timerVal += interval;
    _put32(TIMER_C1, timerVal);

    // enables irqs in the irq controller
    _put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);

    // enables interrupts at the system level
    _enable_interrupts();

    // read which interrupts are waiting
    uint32_t waiting = *((uint32_t*)IRQ_BASIC_PENDING);
    uart_puts(itoa(waiting, 10));
    uart_puts(" <= Basic Pending\n");

    waiting = *((uint32_t*)IRQ_PENDING_1);
    uart_puts(itoa(waiting, 10));
    uart_puts(" <= 1 Pending\n");

    waiting = *((uint32_t*)IRQ_PENDING_2);
    uart_puts(itoa(waiting, 10));
    uart_puts(" <= 2 Pending\n");

}

extern "C" void kernel_main() {
    // set up serial console and linear frame buffer
    uart_init();
    lfb_init();

    // initialize memory and the memory management unit
    mmu_init();

    // initialize interrupts
    init_interrupts();

    // initialize the console now that the lfb has been set
    Console console((uint64_t) lfb);

    // trigger exception
    console.printf("val @ null: %i\n", *((int*) nullptr));
    
    console.printf("lfb: %p\n", lfb);
    console.printf("Current Level: %i\n", getCurrentEL());
    console.printf("_end: %p, _data: %p\n", &_end, &_data);

    // echo everything back
    while (true) {
        console.printf("Clock: %i\n", *(uint32_t*) TIMER_CLO);
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