#include "common/uart.hh"
#include "kernel/regstate.hh"
#include "common/stdlib.hh"

/**
 * exception handler for exceptions that we haven't handled yet ;)
 */
extern "C" void exc_handler(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far) {
    // print out interruption type
    uart_puts(itoa(type, 10));
    uart_puts(") ");
    printf("Exception (%i):\n", type);
    switch(type) {
        case 0: case 4: case 8: uart_puts("Synchronous"); break;
        case 1: case 9: uart_puts("IRQ"); break;
        case 2: uart_puts("FIQ"); break;
        case 3: uart_puts("SError"); break;
        default: {
            uart_puts("UNHANDLED EXCEPTION\n");
            while(1);
        }
    }
    uart_puts(": ");

    // print out daif
    uint64_t daif;
    asm volatile("mrs %0, daif": "=r"(daif));
    uart_puts("\nDAIF: ");
    uart_bin(daif);
    uart_puts("\n");

    printf("ESR: 0x%x\n", esr>>26);
    switch(esr>>26) {
        case 0b000000: uart_puts("Unknown"); break;
        case 0b000001: uart_puts("Trapped WFI/WFE"); break;
        case 0b001110: uart_puts("Illegal execution"); break;
        case 0b010101: uart_puts("System call"); break;
        case 0b100000: uart_puts("Instruction abort, lower EL"); break;
        case 0b100001: uart_puts("Instruction abort, same EL"); break;
        case 0b100010: uart_puts("Instruction alignment fault"); break;
        case 0b100100: uart_puts("Data abort, lower EL"); break;
        case 0b100101: uart_puts("Data abort, same EL"); break;
        case 0b100110: uart_puts("Stack alignment fault"); break;
        case 0b101100: uart_puts("Floating point"); break;
        default: uart_puts("Unknown"); break;
    }

    // decode data abort cause
    if(esr>>26 == 0b100100 || esr>>26 == 0b100101) {
        uart_puts(", ");
        switch((esr>>2) & 0x3) {
            case 0: {
                uart_puts("Address size fault");
                printf("Address size fault");
                break;
            }
            case 1: {
                uart_puts("Translation fault");
                printf("Translation fault");
                break;
            }
            case 2: {
                uart_puts("Access flag fault");
                printf("Access flag fault");
                break;
            }
            case 3: {
                uart_puts("Permission fault");
                printf("Permission fault");
                break;
            }
        }
        uart_puts(" at level ");
        uart_puts(itoa(esr & 0x3, 10));
        uart_puts("\n");
        printf(" at level %i\n", esr & 0x3);
        
    }

    // dump registers
    uart_puts(":\n  ESR_EL1 ");
    uart_hex(esr>>32);
    uart_hex(esr);
    uart_puts(" ELR_EL1 ");
    uart_hex(elr>>32);
    uart_hex(elr);
    uart_puts("\n SPSR_EL1 ");
    uart_hex(spsr>>32);
    uart_hex(spsr);
    uart_puts(" FAR_EL1 ");
    uart_hex(far>>32);
    uart_hex(far);
    uart_puts("\n");

    printf(" ESR_EL1: %p, ELR_EL1: %p\nSPSR_EL1: %p, FAR_EL1: %p\n", esr, elr, spsr, far);
    
    // no return from exception for now
    while(1);
}