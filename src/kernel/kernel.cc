#include "kernel/kernel.hh"
#include "kernel/uart.hh"
#include "kernel/mmu.hh"
#include "kernel/lfb.hh"
#include "kernel/vmiter.hh"
#include "kernel/delays.hh"
#include "kernel/timer.hh"
#include "kernel/proc.hh"

#include "common/types.hh"
#include "common/stdlib.hh"

#include "usr/userland.hh"

extern volatile uint64_t _data;
extern volatile uint64_t _kernel_end;

// a pointer to the first kernel pagetable
pagetable* kernel_pagetable = (pagetable*) &_kernel_end;

// in-memory metadata on memory (that's meta hehe)
pageinfo pages[NPAGES];

// the proc table
proc ptable[NPROC];

proc* current;

// returns the address of the current proc's regs
extern "C" void* get_current_regs() {
    return (void*) &current->regs;
}

[[noreturn]] void run(proc* p);
[[noreturn]] void schedule();
extern "C" void exception_return(proc* p);

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

// handle timer interrupts, interrupts are disabled during this time
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

    // refresh the memviewer
    if (ticks % 10 == 0) {
        memshow();
    }

    exception_return(current);
}

// interrupts are off by default in the syscall_handler
extern "C" [[noreturn]] void syscall_handler(uint16_t syscallno) {
    // place return values in x0, then break to "return"
    switch (syscallno) {
        case SYSCALL_GETPID:
            current->regs.reg_x0 = current->pid;
            break;

        case SYSCALL_YIELD:
            schedule();
            assert(false);
            break;

        default: {
            uart_puts("No such syscall: ");
            uart_puts(itoa(syscallno, 10));
            uart_puts("\n");

            printf("No such syscall: %i\n", syscallno);

            assert(false);
        }
    }
    
    run(current);
}

// the main initialization function for our kernel, interrupts are off in the kernel
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

    // initialize the ptable
    for (pid_t p = 1; p < NPROC; ++p) {
        ptable[p].pid = p;
    }

    // setup the initial userland process
    ptable[1].regs.reg_x0 = 1;
    ptable[1].regs.reg_elr = (uint64_t) &user_main;

    // enable userland interrupts
    ptable[1].regs.reg_spsr = (1<<6) | (0<<7) | (1<<8) | (1<<9);
    
    uint64_t stack_base = (uint64_t) kalloc_page();
    pages[stack_base / PAGESIZE].owner = 1;
    ptable[1].regs.reg_sp = stack_base + PAGESIZE;

    ptable[1].pt = kernel_pagetable;
    ptable[1].state = P_RUNNABLE;

    // doesn't return
    run(&ptable[1]);

    // shouldn't get here
    assert(false);
}    

// checks some basics about the process, then runs process p
void run(proc* p) {
    assert(p->state == P_RUNNABLE);
    current = p;

    exception_return(p);

    assert(false);
}

// a basic scheduler that picks the next runnable process and runs it
void schedule() {
    pid_t idx;
    for (pid_t pid_off = 0; pid_off < NPROC; ++pid_off) {
        idx = (current->pid + pid_off + 1) % NPROC;
        if (ptable[idx].state == P_RUNNABLE) {
            run(&ptable[idx]);
        }
    }

    assert(false);
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
            && !pages[pa / PAGESIZE].used()
        ) {
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
    fbInfo.ypos = ((fbInfo.height / font->height) / 2);

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