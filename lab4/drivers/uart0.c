#include "uart0.h"
#include "mailbox.h"
#include "gpio.h"
#include "circular_buffer.h"
#include "irq.h"
#include "string.h"

static struct circular_buffer rx_buffer;
static struct circular_buffer tx_buffer;

void uart0_init()
{
	uint32_t val;
	
	circular_buffer_init(&rx_buffer);
	circular_buffer_init(&tx_buffer);

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

	//Enable UART0 tx and rx interrupt.
	val = UART0_IMSC_TXIM | UART0_IMSC_RXIM;
	writel(UART0_IMSC_REG, val);

	//Set interrupt trigger level
	val = readl(UART0_IFLS_REG);
	val &= ~UART0_IFLS_RXIFLSEL_MASK;
	val &= ~UART0_IFLS_TXIFLSEL_MASK;
	val |= UART0_IFLS_RXIFLSEL(UART0_IFLS_IFLSEL_1_2);
	val |= UART0_IFLS_TXIFLSEL(UART0_IFLS_IFLSEL_1_2);
	writel(UART0_IFLS_REG, val);

	//Set 8 bits mode and enable FIFO
	val = readl(UART0_LCRH_REG);
	val &= ~UART0_LCRH_WLEN_MASK;
	val |= UART0_LCRH_WLEN_8BITS;
	val |= UART0_LCRH_FEN;
	writel(UART0_LCRH_REG, val);

	val = UART0_CR_ENABLE_UART | UART0_CR_ENABLE_TX | UART0_CR_ENABLE_RX;
	writel(UART0_CR_REG, val);

	irq_enable_arm_peri(ARM_PERI_IRQ_NUM_UART);
}

char uart0_getc()
{
#if 0
	char c;
	uint32_t val;
	do {
		val = readl(UART0_FR_REG);
		asm volatile("nop");
	} while ((val & UART0_FR_RXFE));

	c = (char) readl(UART0_DR_REG);

	return c == '\r' ? '\n' : c;
#else
	char c;

	while (circular_buffer_empty(&rx_buffer)) {
		asm volatile("nop");
	}

	c = (char)circular_buffer_read(&rx_buffer);

	return c == '\r' ? '\n' : c;
#endif
}

void uart0_putc(unsigned char c)
{
#if 0
	uint32_t val;
	do {
		val = readl(UART0_FR_REG);
		asm volatile("nop");
	} while ((val & UART0_FR_TXFF));

	writel(UART0_DR_REG, c);
#else
	// Disable TX interrupt temporarily
	uint32_t imsc = readl(UART0_IMSC_REG);
	writel(UART0_IMSC_REG, imsc & ~UART0_IMSC_TXIM);
	
	// Add character to buffer
	circular_buffer_write(&tx_buffer, c);
	
	// If TX FIFO has space, kickstart transmission
	if (!(readl(UART0_FR_REG) & UART0_FR_TXFF)) {
		
		if (!circular_buffer_empty(&tx_buffer)) {
			c = (char)circular_buffer_read(&tx_buffer);
			writel(UART0_DR_REG, c);
		}
	}
	
	// Re-enable TX interrupt
	writel(UART0_IMSC_REG, imsc);
#endif
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
#if 0	
	char c;
	uint32_t val;
	do {
		val = readl(UART0_FR_REG);
		asm volatile("nop");
	} while ((val & UART0_FR_RXFE));

	c = (char) readl(UART0_DR_REG);

	return c;
#else
	char c;
	
	while (circular_buffer_empty(&rx_buffer)) {
		asm volatile("nop");
	}

	c = (char)circular_buffer_read(&rx_buffer);

	return c;
#endif
}

void uart0_do_rx()
{
	char c;

	while (!(readl(UART0_FR_REG) & UART0_FR_RXFE)) {
		c = (char) readl(UART0_DR_REG);
		circular_buffer_write(&rx_buffer, c);
	}
}

void uart0_handle_irq()
{
	char c;
	uint32_t mis = readl(UART0_MIS_REG);
	
	if (mis & UART0_MIS_RXMIS) {
		uart0_do_rx();
		writel(UART0_ICR_REG, UART0_ICR_RXIC);
	}

	if (mis & UART0_MIS_TXMIS) {
		// Fill TX FIFO with data from buffer
		while (!circular_buffer_empty(&tx_buffer) && 
			!(readl(UART0_FR_REG) & UART0_FR_TXFF)) {
			c = (char)circular_buffer_read(&tx_buffer);
			writel(UART0_DR_REG, c);
		}

		// Clear TX interrupt
		writel(UART0_ICR_REG, UART0_ICR_TXIC);
	}
}