#include "kernel/uart.hh"
#include "kernel/lfb.hh"

#include "common/types.hh"
#include "common/stdlib.hh"

extern "C" void kernel_main() {
    // set up serial console and linear frame buffer
    uart_init();
    lfb_init();

    // display an ASCII string on screen with PSF
    lfb_print(80, 80, "Hello World!");
    lfb_putc(80, 120, 'A', 0x000000, 0xFFFFFF);

    // echo everything back
    while (true) {
        int c = toupper(uart_getc());
        uart_putc(c);
    }
}
