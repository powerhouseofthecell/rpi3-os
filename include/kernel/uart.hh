#ifndef UART_HH
#define UART_HH

void uart_init();
void uart_putc(char c);
char uart_getc();
void uart_puts(const char* s);
void uart_hex(unsigned int d);

#endif