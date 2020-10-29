#include "kernel/uart.hh"
#include "kernel/lfb.hh"

extern "C" void kernel_main() {
    // set up serial console and linear frame buffer
    uart_init();
    lfb_init();

    // display an ASCII string on screen with PSF
    lfb_print(80, 80, "Hello World!");

    // display a UTF-8 string on screen with SSFN
    lfb_proprint(80, 120, "Please for the love of god boot");

    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
}
