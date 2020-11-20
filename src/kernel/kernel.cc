#include "kernel/kernel.hh"
#include "kernel/uart.hh"
#include "kernel/mmu.hh"
#include "kernel/lfb.hh"
#include "kernel/vmiter.hh"
#include "kernel/delays.hh"
#include "kernel/timer.hh"

#include "common/types.hh"
#include "common/stdlib.hh"

extern volatile uint64_t _data;
extern volatile uint64_t _kernel_end;

// a pointer to the first kernel pagetable
pagetable* kernel_pagetable = (pagetable*) &_kernel_end;

// in-memory metadata on memory (that's meta hehe)
pageinfo pages[NPAGES];

// call and initialize cpp constructors
void constructors_init() {
    typedef void (*constructor_function)();
    extern constructor_function __init_array_start[];
    extern constructor_function __init_array_end[];
    for (auto fp = __init_array_start; fp != __init_array_end; ++fp) {
        printf("%p\n", fp);
        (*fp)();
    }
}

// handle timer interrupts
extern "C" void irq_handler() {
    // increment our "clock's" ticks
    // reset after (hr * 60 min/hr * 60 sec/min * HZ irq/sec) irqs
    ticks = (ticks + 1) % (1 * 60 * 60 * HZ);

    // reset the timer
    // TODO: #define the magic numbers here
    *((uint32_t*) 0x40000038) = (uint32_t) (LOCAL_TIMER_RELOAD | (1<<30));

    // restore the cursor position after this printing
    uint32_t old_x = fbInfo.xpos;
    uint32_t old_y = fbInfo.ypos;
    puts(fbInfo.width / font->width - 12, 0, "Tick: ", BLACK, WHITE);
    puts(fbInfo.width / font->width - 6, 0, itoa(ticks, 10), BLACK, WHITE);
    fbInfo.xpos = old_x;
    fbInfo.ypos = old_y;

    // every tick, refresh the memviewer
    memshow();
}

// the main initialization function for our kernel
extern "C" void kernel_main() {
    // initialize the cpp constructors
    constructors_init();

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
    printf("&_kernel_end: %p, &_data: %p\n", &_kernel_end, &_data);
    printf("pages[1].refcount? %i\n", pages[1].refcount);
    printf("w: %i, h: %i\n", font->width, font->height);

    // loop forever
    while (true) {

    }
}

// return true iff pa is reserved for special uses (MMIO or nullpage)
bool reserved_physical_address(uintptr_t pa) {
    return pa < PAGESIZE || (pa >= MMIO_BASE && pa < MMIO_END);
}

// allocatable_physical_address(pa)
//    Returns true iff `pa` is an allocatable physical address, i.e.,
//    not reserved or holding kernel data.

bool allocatable_physical_address(uintptr_t pa) {
    return !reserved_physical_address(pa)
        && (pa < KERNEL_START_ADDR
            || pa >= round_up((uintptr_t) &_kernel_end, PAGESIZE))
        && (pa < KERNEL_STACK_TOP - PAGESIZE
            || pa >= KERNEL_STACK_TOP)
        && pa < MEMSIZE_PHYSICAL;
}

// allocate a page of memory in the kernel
void* kalloc_page() {
    for (uintptr_t pa = 0; pa < MEMSIZE_PHYSICAL; pa += PAGESIZE) {
        if (allocatable_physical_address(pa)
            && !pages[pa / PAGESIZE].used()) {
            ++pages[pa / PAGESIZE].refcount;
            memset((void*) pa, 0xAA, PAGESIZE);
            return (void*) pa;
        }
    }
    return nullptr;
}

// free a provided page of memory in the kernel
void kfree_page(void* pa) {
    // do nothing for nullptr or non-page addresses
    if (pa == nullptr || ((uint64_t) pa) % PAGESIZE != 0) {
        return;
    }

    if (allocatable_physical_address((uintptr_t) pa)
        && pages[(uint64_t)pa / PAGESIZE].used()
    ) {
        --pages[(uint64_t)pa / PAGESIZE].refcount;
    }
}

// draw a depiction of the state of memory to the screen, starting at halfway down
void memshow() {
    uint32_t old_xpos = fbInfo.xpos;
    uint32_t old_ypos = fbInfo.ypos;

    fbInfo.xpos = 0;
    fbInfo.ypos = ((fbInfo.height / font->height) / 2) - 4;

    printf("PHYSICAL MEMORY");

    uint32_t pages_per_row = 32;
    int color_idx = 0;
    for (uint64_t pa = 0x00, page_num = 0; pa < MEMSIZE_PHYSICAL; pa += PAGESIZE, ++page_num) {
        // on each new row
        if (page_num % pages_per_row == 0) {
            printf("\n%p\t", pa);
        }

        color_idx = pages[page_num].owner;

        puts(" ");
        if (pages[page_num].used()) {
            putc(CONSOLE_SQUARE, COLORS[color_idx], BLACK);
        } else {
            putc(CONSOLE_SQUARE_OUTLINE, WHITE, BLACK);
        }
    }

    fbInfo.xpos = old_xpos;
    fbInfo.ypos = old_ypos;
}