#ifndef _UART0_H
#define _UART0_H

#include "core.h"

#define UART0_BASE		(MMIO_BASE + 0x00201000)

#define UART0_DR_REG				(UART0_BASE + 0x0)
#define UART0_FR_REG				(UART0_BASE + 0x18)
#define   UART0_FR_TXFE		BIT(7)
#define   UART0_FR_RXFF		BIT(6)
#define   UART0_FR_TXFF		BIT(5)
#define   UART0_FR_RXFE		BIT(4)

#define UART0_IBRD_REG				(UART0_BASE + 0x24)
#define UART0_FBRD_REG				(UART0_BASE + 0x28)

#define UART0_LCRH_REG				(UART0_BASE + 0x2C)
#define   UART0_LCRH_WLEN_MASK		GENMASK(6, 5)
#define   UART0_LCRH_WLEN_5BITS		(0 << 5)
#define   UART0_LCRH_WLEN_6BITS		(1 << 5)
#define   UART0_LCRH_WLEN_7BITS		(2 << 5)
#define   UART0_LCRH_WLEN_8BITS		(3 << 5)
#define   UART0_LCRH_FEN			BIT(4)


#define UART0_CR_REG			(UART0_BASE + 0x30)
#define   UART0_CR_DISABLE_ALL	0x0
#define   UART0_CR_ENABLE_UART	BIT(0)
#define   UART0_CR_ENABLE_TX	BIT(8)
#define   UART0_CR_ENABLE_RX	BIT(9)

#define UART0_IMSC_REG		(UART0_BASE + 0x38)

#define UART0_ICR_REG			(UART0_BASE + 0x44)
#define   UART0_ICR_CLEAR_ALL	GENMASK(10, 0)

#define UART0_DEFAULT_CLK_RATE	4000000		//4MHZ

void uart0_init();

char uart0_getc();
void uart0_putc(unsigned char c);
void uart0_puts(const char *s);
void uart0_flush();
char uart0_getraw();

#endif