.section ".text.boot"

.global _start
_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    // set stack before our code
    ldr     x1, =_start
    mov     sp, x1

    // get the current exception level
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    // check that we're running at EL1
    cmp     x0, #12
    bne     5f

    // should never be executed, just for completeness
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, 5f
    msr     elr_el3, x2
    eret

    // running at EL2?
5:  cmp     x0, #4
    beq     5f
    msr     sp_el1, x1
    // enable CNTP for EL1
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr
    // enable AArch64 in EL1
    mov     x0, #(1 << 31)      // AArch64
    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
    msr     hcr_el2, x0
    mrs     x0, hcr_el2
    // Setup SCTLR access
    mov     x2, #0x0800
    movk    x2, #0x30d0, lsl #16
    msr     sctlr_el1, x2
    // setup exception handler
    ldr     x2, =_vectors
    msr     vbar_el1, x2
    // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2
    eret

5:  mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to CPP code, should not return
4:  bl      kernel_main
    b       1b

// a macro to create vector entries
.macro ventry idx label
    .align    7
    mov     x0, #\idx
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       \label
.endm

// vectors for exception handling
.align 11
_vectors:
    // synchronous
    ventry 0, exc_handler

    // IRQ
    ventry 1, exc_handler

    // FIQ
    ventry 2, exc_handler

    // SError
    ventry 3, exc_handler

    // other handlers/exceptions
    ventry 4,   exc_handler
    ventry 5,   exc_handler
    ventry 6,   exc_handler
    ventry 7,   exc_handler
    ventry 8,   exc_handler
    ventry 9,   exc_handler
    ventry 10,  exc_handler
    ventry 11,  exc_handler
    ventry 12,  exc_handler
    ventry 13,  exc_handler
    ventry 14,  exc_handler
    ventry 15,  exc_handler

.global _enable_interrupts
// turns on global interrupts
_enable_interrupts:
    msr daifclr, #2
    ret

.global _disable_interrupts
// turns off global interrupts
_disable_interrupts:
    msr daifset, #2
    ret

.globl _put32
_put32:
	str w1,[x0]
	ret

.globl _get32
_get32:
	ldr w0,[x0]
	ret
