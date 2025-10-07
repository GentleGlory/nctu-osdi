#include "mini_uart.h"
#include "gpio.h"

void mini_uart_init()
{
	uint32_t val;

	//Set GPIO14 and GPIO15 with alt function 5 and pull none
	gpio_pin_sel_fn(14, GPF_FUNC_5);
	gpio_pin_sel_fn(15, GPF_FUNC_5);
	gpio_pin_set_pud(14, PULL_TYPE_NONE);
	gpio_pin_set_pud(15, PULL_TYPE_NONE);

	val = readl(AUX_AUXENB_REG);
	val |= AUX_AUXENB_MINI_UART;
	writel(AUX_AUXENB_REG, val);

	writel(AUX_MU_CNTL_REG, 0);
	writel(AUX_MU_IER_REG, 0);

	val = readl(AUX_MU_LCR_REG);
	val &= ~AUX_MU_LCR_DATA_BIT_MASK;
	val |= AUX_MU_LCR_DATA_8_BIT;
	writel(AUX_MU_LCR_REG, val);

	writel(AUX_MU_MCR_REG, 0);

	//Set baud_rate to 115200
	//baud_rate = system_clock / ( 8 * (AUX_MU_BAUD + 1))
	//system_clock is 250 MHZ
	writel(AUX_MU_BAUD, 270);

	val = AUX_MU_IIR_CLEAR_RX_FIFO | AUX_MU_IIR_CLEAR_TX_FIFO;
	writel(AUX_MU_IIR_REG, 0xc6);

	val = AUX_MU_CNTL_RX_EN | AUX_MU_CNTL_TX_EN;
	writel(AUX_MU_CNTL_REG, val);
}

char mini_uart_getc()
{
	char c;
	uint32_t val;
	do {
		val = readl(AUX_MU_LSR_REG);
		asm volatile("nop");
	} while (!(val & AUX_MU_LSR_DATA_READY));

	c = (char) readl(AUX_MU_IO_REG);

	return c == '\r' ? '\n' : c;
}

void mini_uart_putc(unsigned char c)
{
	uint32_t val;
	do {
		val = readl(AUX_MU_LSR_REG);
		asm volatile("nop");
	} while (!(val & AUX_MU_LSR_TX_EMPTY));

	writel(AUX_MU_IO_REG, c);
}

void mini_uart_puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			mini_uart_putc('\r');
		
		mini_uart_putc(*s++);
	}
}

void mini_uart_flush()
{
	while (readl(AUX_MU_LSR_REG) & AUX_MU_LSR_DATA_READY) {
		readl(AUX_MU_IO_REG);
	}
}
