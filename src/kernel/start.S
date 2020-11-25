#include "kernel/start.hh"

.section ".text.boot"

.global _start
_start:
    // read cpu id
    mrs     x1, mpidr_el1
    and     x1, x1, #0xFF
    cbz     x1, _setup_begin

    // cpu id > 0, stop
    b       halt

_setup_begin:
    // get the current exception level
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    // check that we're running at EL1
    cmp     x0, #12
    bne     _setup_sys_regs

    // should never be executed, just in case
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, _setup_sys_regs
    msr     elr_el3, x2
    eret

// setup important system registers: SCTLR, HCR, SPSR, VBAR
_setup_sys_regs:
    // SCTLR
    ldr     x2, =SCTLR_EL1_VAL
    msr     sctlr_el1, x2

    // HCR - AArch64 in EL1
    ldr     x0, =HCR_EL2_VAL
    msr     hcr_el2, x0

    // SPSR - change execution level to EL1
    ldr     x2, =SPSR_EL2_VAL
    msr     spsr_el2, x2

    // VBAR - exception handlers
    ldr     x2, =_vectors
    msr     vbar_el1, x2

    // "return address" for our eret (drops exception level)
    adr     x2, _setup_end
    msr     elr_el2, x2

    eret

// finish setup and jump to the kernel
_setup_end:  
    // clear bss
    ldr     x0, =__bss_start
    ldr     x1, =__bss_size
    bl      memzero

    // set stack before our code
    ldr     x1, =__kernel_stack_top
    mov     sp, x1

    // jump to the kernel (shouldn't return)
    bl      kernel_main

    // halt this core in case kernel_main returns
    b       halt

// zeros sz bytes (x1) of memory starting at addr (x0)
// NOTE: goes by 8 byte increments
memzero:
    str     xzr, [x0], #8
    sub     x1, x1, #8
    cbnz    x1, memzero

    ret

// puts this cpu into low power mode
halt:
    b halt

// turns on global interrupts
.global enable_interrupts
enable_interrupts:
    msr daifclr, #2
    ret

// turns off global interrupts
.global disable_interrupts
disable_interrupts:
    msr daifset, #2
    ret

// save state
.macro	kernel_entry    
    // save general purpose registers
	stp	x0, x1,   [sp, #-16]!
	stp	x2, x3,   [sp, #-16]!
	stp	x4, x5,   [sp, #-16]!
	stp	x6, x7,   [sp, #-16]!
	stp	x8, x9,   [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x14, x15, [sp, #-16]!
	stp	x16, x17, [sp, #-16]!
	stp	x18, x19, [sp, #-16]!
	stp	x20, x21, [sp, #-16]!
	stp	x22, x23, [sp, #-16]!
	stp	x24, x25, [sp, #-16]!
	stp	x26, x27, [sp, #-16]!
	stp	x28, x29, [sp, #-16]!
	stp	x30, xzr, [sp, #-16]!

    // load special system registers
    mrs x0, tpidr_el0
    mrs x1, sp_el0
    mrs x2, spsr_el1
    mrs x3, elr_el1
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!

    // load the kernel pagetable
    ldr x0, =_kernel_end
    msr ttbr0_el1, x0

    // pass the stack pointer as the first arg
    mov x0, sp
.endm

// restore state
.macro	kernel_exit
    // restore special system registers
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    msr elr_el1,    x3
    msr spsr_el1,   x2
    msr sp_el0,     x1
    msr tpidr_el0,  x0

    // restore general purpose registers
    ldp	x30, xzr, [sp], #16
    ldp	x28, x29, [sp], #16
    ldp	x26, x27, [sp], #16
    ldp	x24, x25, [sp], #16
    ldp	x22, x23, [sp], #16
    ldp	x20, x21, [sp], #16
    ldp	x18, x19, [sp], #16
    ldp	x16, x17, [sp], #16
    ldp	x14, x15, [sp], #16
    ldp	x12, x13, [sp], #16
    ldp	x10, x11, [sp], #16
    ldp	x8, x9,   [sp], #16
    ldp	x6, x7,   [sp], #16
    ldp	x4, x5,   [sp], #16
    ldp	x2, x3,   [sp], #16
	ldp	x0, x1,   [sp], #16
	eret
.endm

.global exception_return
exception_return:
    // proc* in x0
    // load this proc's pagetable
    ldr x1, [x0]
    msr ttbr0_el1, x1

    // proc* + 16 is regs
    add x1, x0, #16
    mov x4, x1

    // restore special system registers
    ldp x2, x3, [x4], #16
    ldp x0, x1, [x4], #16
    msr elr_el1,    x3
    msr spsr_el1,   x2
    msr sp_el0,     x1
    msr tpidr_el0,  x0

    mov x1, x4

    // restore general purpose registers
    ldp	x30, xzr, [x1], #16
    ldp	x28, x29, [x1], #16
    ldp	x26, x27, [x1], #16
    ldp	x24, x25, [x1], #16
    ldp	x22, x23, [x1], #16
    ldp	x20, x21, [x1], #16
    ldp	x18, x19, [x1], #16
    ldp	x16, x17, [x1], #16
    ldp	x14, x15, [x1], #16
    ldp	x12, x13, [x1], #16
    ldp	x10, x11, [x1], #16
    ldp	x8, x9,   [x1], #16
    ldp	x6, x7,   [x1], #16
    ldp	x4, x5,   [x1], #16
    ldp	x2, x3,   [x1], #16
	ldp	x0, x1,   [x1]
	eret

// a macro to create vector entries
.macro ventry label
    .balign  0x80
    b      \label
.endm

// vectors for exception handling
.balign 0x800
_vectors:
    ventry unhandled_exception_0
    ventry unhandled_exception_1
    ventry unhandled_exception_2
    ventry unhandled_exception_3

    ventry unhandled_exception_4
    ventry irq_wrapper
    ventry unhandled_exception_6
    ventry unhandled_exception_7

    ventry unhandled_exception_8
    ventry irq_wrapper
    ventry unhandled_exception_10
    ventry unhandled_exception_11

    ventry unhandled_exception_12
    ventry unhandled_exception_13
    ventry unhandled_exception_14
    ventry unhandled_exception_15

// a wrapper for handling irqs
irq_wrapper:
	kernel_entry
	b	irq_handler

// a block of exception handlers for unimplemented exceptions
unhandled_exception_0:
    mov     x0, #0
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler

unhandled_exception_1:
    mov     x0, #1
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler

unhandled_exception_2:
    mov     x0, #2
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler

unhandled_exception_3:
    mov     x0, #3
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler

unhandled_exception_4:
    mov     x0, #4
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_5:
    mov     x0, #5
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_6:
    mov     x0, #6
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_7:
    mov     x0, #7
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_8:
    mov     x0, #8
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_9:
    mov     x0, #9
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_10:
    mov     x0, #10
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_11:
    mov     x0, #11
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_12:
    mov     x0, #12
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_13:
    mov     x0, #13
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_14:
    mov     x0, #14
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
unhandled_exception_15:
    mov     x0, #15
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       exc_handler
