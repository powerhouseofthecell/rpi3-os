#include "kernel/gpio.hh"
#include "kernel/uart.hh"
#include "kernel/mmu.hh"
#include "kernel/pagetable.hh"
#include "kernel/vmiter.hh"

#define PAGESIZE    4096

// granularity
#define PT_PAGE     0b11        // 4k granule
#define PT_BLOCK    0b01        // 2M granule
// accessibility
#define PT_KERNEL   (0<<6)      // privileged, supervisor EL1 access only
#define PT_USER     (1<<6)      // unprivileged, EL0 access allowed
#define PT_RW       (0<<7)      // read-write
#define PT_RO       (1<<7)      // read-only
#define PT_AF       (1<<10)     // accessed flag
#define PT_NX       (1UL<<54)   // no execute
// shareability
#define PT_OSH      (2<<8)      // outter shareable
#define PT_ISH      (3<<8)      // inner shareable
// defined in MAIR register
#define PT_MEM      (0<<2)      // normal memory
#define PT_DEV      (1<<2)      // device MMIO
#define PT_NC       (2<<2)      // non-cachable

#define TTBR_CNP    1

// get addresses from linker
extern volatile unsigned char _data;
extern volatile unsigned char _end;

/**
 * Set up page translation tables and enable virtual memory
 */
void mmu_init()
{
    unsigned long data_page = (unsigned long) &_data / PAGESIZE;
    unsigned long r, b;

    // all initial pagetables for the kernel live starting at _end
    pagetable* pagetables = (pagetable*) &_end;

    /* setup initial pagetables (identity mapped in user/EL0-addresses) */
    // NOTE: addresses here are physical because MMU has not been switched on yet

    // TTBR0, identity L1
    pagetables[0].entry[0] = (pageentry_t) (&pagetables[2]) |    // physical address
        PTE_PAGE|
        PTE_A   |     // accessed flag. Without this we're going to have a Data Abort exception
        PTE_PWU |     // non-privileged
        PT_ISH;       // inner shareable

    // identity L2, first 2M block : our OS assumes this is all of physical memory
    pagetables[2].entry[0] = (pageentry_t) (&pagetables[3]) | // physical address
        PTE_PAGE|
        PTE_A   |     // accessed flag
        PTE_PWU |     // non-privileged
        PT_ISH;       // inner shareable

    // identity L2 2M blocks
    b = MMIO_BASE >> SECTION_SHIFT;
    // skip 0th, since that's above
    for (r = 1; r < 512; r++) {
        pagetables[2].entry[r] = (pageentry_t) ((r << SECTION_SHIFT)) |  // physical address
        PTE_BLOCK|
        PTE_P    |    // map 2M block
        PTE_A    |    // accessed flag
        PTE_U    |    // non-privileged
        (r >= b ? PT_OSH | PT_DEV : PT_ISH | PT_MEM); // different attributes for device memory
    }

    // identity L3, skipping 0x0 for debugging
    pagetables[3].entry[0] = (pageentry_t) 0x0;
    for (r = 1; r < 512; r++) {
        if (r < 0x80 || r >= data_page) {
            pagetables[3].entry[r] = (pageentry_t) (r * PAGESIZE) |   // physical address
                PTE_PAGE|
                PTE_P   |
                PTE_U   |
                PTE_A   |   // accessed flag
                PT_ISH  |   // inner shareable
                PTE_W;      // writeable
        } else {
            pagetables[3].entry[r] = (pageentry_t) (r * PAGESIZE) |   // physical address
                PTE_PAGE|
                PTE_P   |
                PTE_U   |
                PTE_A   |   // accessed flag
                PT_ISH  |   // inner shareable
                PTE_R;      // read-only
        }
        
    }

    // TTBR1, kernel L1 @ index 511
    pagetables[1].entry[511] = (pageentry_t) (&pagetables[4]) | // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PTE_A   |     // accessed flag
        PT_ISH  |     // inner shareable
        PT_MEM;       // normal memory

    // kernel L2 @ index 511
    pagetables[4].entry[511] = (pageentry_t) (&pagetables[5]) |   // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PTE_A   |     // accessed flag
        PT_ISH  |     // inner shareable
        PT_MEM;       // normal memory

    // kernel L3 @ index 0: map a page of MMIO addresses
    pagetables[5].entry[0] = (pageentry_t) (MMIO_BASE + 0x00201000) |   // physical address
        PT_PAGE |     // map 4k
        PTE_A   |     // accessed flag
        PT_NX   |     // no execute
        PT_OSH  |     // outter shareable
        PT_DEV;       // device memory

    // kernel L3 @ index 1: map a page of the stack
    pagetables[5].entry[1] = (pageentry_t) (0x7F000) |   // physical address
        PT_PAGE |     // map 4k
        PTE_A   |     // accessed flag
        PT_NX   |     // no execute
        PT_ISH;       // outter shareable

    /* SECTION: setup system registers to enable MMU */

    // check for 4k granule and at least 36 bits physical address bus */
    asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));

    b = r & 0xF;
    if(r & (0xF << 28) /* 4k */ || b < 1 /* 36 bits */) {
        uart_puts("ERROR: 4k granule or 36 bit address space not supported\n");
        return;
    }

    // first, set Memory Attributes array, indexed by PT_MEM, PT_DEV, PT_NC in our example
    r = (0xFF << 0) |    // AttrIdx=0: normal, IWBWA, OWBWA, NTR
        (0x04 << 8) |    // AttrIdx=1: device, nGnRnE (must be OSH too)
        (0x44 << 16);    // AttrIdx=2: non cacheable
    asm volatile ("msr mair_el1, %0" : : "r" (r));

    // check if hardware can manage the access or dirty flags
    // unsigned long q;
    // asm volatile("mrs %0, ID_AA64MMFR1_EL1": "=r" (q));
    // if (q & 0b1111 == 0) {
    //     uart_puts("no support\n");
    // } else {
    //     uart_puts("at least some support\n");
    // }

    // next, specify mapping characteristics in translate control register
    //TCR_T0SZ | TCR_T1SZ;
    r = (0b1LL << 40)   | // HD=1, turn on hardware management of dirty
        (0b1LL << 39)   | // HA=1, turn on hardware management of access flag
        (0b00LL << 37)  | // TBI=0, no tagging
        (b << 32)       | // IPS=autodetected
        TCR_TG1_4K      | // TG1=4k
        (0b11LL << 28)  | // SH1=3 inner
        (0b01LL << 26)  | // ORGN1=1 write back
        (0b01LL << 24)  | // IRGN1=1 write back
        (0b0LL  << 23)  | // EPD1 enable higher half
        (25LL   << 16)  | // T1SZ=25, 3 levels (512G)
        TCR_TG0_4K      | // TG0=4k
        (0b11LL << 12)  | // SH0=3 inner
        (0b01LL << 10)  | // ORGN0=1 write back
        (0b01LL << 8)   | // IRGN0=1 write back
        (0b0LL  << 7)   | // EPD0 enable lower half
        (25LL   << 0);    // T0SZ=25, 3 levels (512G)
    asm volatile ("msr tcr_el1, %0; isb" : : "r" (r));

    // tell the MMU where our translation tables are
    // lower half, user space
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((unsigned long) &pagetables[0]));
    
    // upper half, kernel space
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((unsigned long) &pagetables[1]));

    // finally, toggle some bits in system control register to enable page translation
    asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (r));
    
    r |= 0xC00800;     // set mandatory reserved bits
    r &= ~((1<<25) |   // clear EE, little endian translation tables
         (1<<24)   |   // clear E0E
         (1<<19)   |   // clear WXN
         (1<<12)   |   // clear I, no instruction cache
         (1<<4)    |   // clear SA0
         (1<<3)    |   // clear SA
         (1<<2)    |   // clear C, no cache at all
         (1<<1));      // clear A, no aligment check

    r |= (1<<0);       // set M, enable MMU

    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));
}