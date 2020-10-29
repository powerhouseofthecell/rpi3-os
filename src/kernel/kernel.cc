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

    // display a UTF-8 string on screen with SSFN
    lfb_proprint(80, 120, "Bless this little OS!");

    // echo everything back
    while(1) {
        uint8_t c = (uint8_t) toupper(uart_getc());
        uart_send((unsigned int) c);
    }
}
