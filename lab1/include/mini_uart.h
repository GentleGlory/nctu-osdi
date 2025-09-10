#ifndef _MINI_UART_H
#define _MINI_UART_H

#include "aux_peri.h"


void mini_uart_init();

char mini_uart_getc();
void mini_uart_putc(unsigned char c);
void mini_uart_puts(const char *s);

#endif