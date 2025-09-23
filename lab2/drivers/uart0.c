#include "uart0.h"
#include "mailbox.h"
#include "gpio.h"



void uart0_init()
{
	uint32_t val;
	//Disable UART0
	writel(UART0_CR_REG, UART0_CR_DISABLE_ALL);

	mailbox_set_clock_rate(MAILBOX_CLK_ID_UART, UART0_DEFAULT_CLK_RATE, 0);

	gpio_pin_sel_fn(14, GPF_FUNC_0);
	gpio_pin_sel_fn(15, GPF_FUNC_0);
	gpio_pin_set_pud(14, PULL_TYPE_NONE);
	gpio_pin_set_pud(15, PULL_TYPE_NONE);

	writel(UART0_ICR_REG, UART0_ICR_CLEAR_ALL);
	// BAUDDIV = 4000000 / (16 * 115200) = 2.17
	// BAUDDIV = IBRD + (FBRD/64)
	// IBRD = 2, FBRD = 0.170138888 * 64 = 10.88888 round off to 0xb
	writel(UART0_IBRD_REG, 2);
	writel(UART0_FBRD_REG, 0xb);

	val = readl(UART0_LCRH_REG);
	val &= ~UART0_LCRH_WLEN_MASK;
	val |= UART0_LCRH_WLEN_8BITS;
	val |= UART0_LCRH_FEN;
	writel(UART0_LCRH_REG, val);

	val = UART0_CR_ENABLE_UART | UART0_CR_ENABLE_TX | UART0_CR_ENABLE_RX;
	writel(UART0_CR_REG, val);
}

char uart0_getc()
{
	char c;
	uint32_t val;
	do {
		val = readl(UART0_FR_REG);
		asm volatile("nop");
	} while ((val & UART0_FR_RXFE));

	c = (char) readl(UART0_DR_REG);

	return c == '\r' ? '\n' : c;
}

void uart0_putc(unsigned char c)
{
	uint32_t val;
	do {
		val = readl(UART0_FR_REG);
		asm volatile("nop");
	} while ((val & UART0_FR_TXFF));

	writel(UART0_DR_REG, c);
}

void uart0_puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			uart0_putc('\r');
		
		uart0_putc(*s++);
	}
}

void uart0_flush()
{
	while (!(readl(UART0_FR_REG) & UART0_FR_RXFE)) {
		readl(UART0_DR_REG);
	}
}

char uart0_getraw()
{
	char c;
	uint32_t val;
	do {
		val = readl(UART0_FR_REG);
		asm volatile("nop");
	} while ((val & UART0_FR_RXFE));

	c = (char) readl(UART0_DR_REG);

	return c;
}