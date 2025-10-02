#ifndef _UART_H
#define _UART_H

enum UART_TYPE {
	UART_TYPE_MINI_UART,
	UART_TYPE_UART0,
	UART_TYPE_TOTAL
};

void uart_init(enum UART_TYPE type);

char uart_getc();
void uart_putc(unsigned char c);
void uart_puts(const char *s);
void uart_flush();
char uart_getraw();
void uart_handle_irq(enum UART_TYPE type);
void uart_do_rx();

#endif