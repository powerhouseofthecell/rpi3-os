#include "kernel/uart.hh"
#include "kernel/lfb.hh"
#include "common/console.hh"

#include "common/types.hh"
#include "common/stdlib.hh"

extern "C" void kernel_main() {
    // set up serial console and linear frame buffer
    uart_init();
    lfb_init();

    Console console((uint64_t) lfb);
    uint8_t pixel[3] = {0xFF, 0xFF, 0xFF};
    console.write_pixel(0, 0, pixel);

    // display an ASCII string on screen with PSF
    lfb_print(80, 80, "Hello World!");
    lfb_putc(80, 120, 'A', 0x000000, 0xFFFFFF);

    // echo everything back
    while (true) {
        int c = toupper(uart_getc());
        uart_putc(c);
    }
}
