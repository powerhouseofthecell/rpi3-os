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
    console.puts("Hello, world!\n");

    // echo everything back
    while (true) {
        int c = toupper(uart_getc());
        uart_putc(c);
    }
}
