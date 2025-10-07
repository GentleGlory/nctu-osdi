#include "uart.h"
#include "uart0.h"
#include "mini_uart.h"

typedef void (*init_pfn)(void);
typedef char (*getc_pfn)(void);
typedef void (*putc_pfn)(unsigned char c);
typedef void (*puts_pfn)(const char *s);
typedef void (*flush_pfn)(void);
typedef char (*getraw_pfn)(void);
typedef void (*handle_irq_pfn)(void);
typedef void (*do_rx_pfn)(void);

struct uart_instance {
	init_pfn		init;
	getc_pfn		getc;
	putc_pfn		putc;
	puts_pfn		puts;
	flush_pfn		flush;
	getraw_pfn		getraw;
	handle_irq_pfn	handle_irq;
	do_rx_pfn		do_rx;
};

static enum UART_TYPE cur_uart_type = UART_TYPE_MINI_UART;

static const struct uart_instance uart_instance[UART_TYPE_TOTAL] = {
	{
		.init = mini_uart_init,
		.getc = mini_uart_getc,
		.putc = mini_uart_putc,
		.puts = mini_uart_puts,
		.flush = mini_uart_flush,
	},
	{
		.init = uart0_init,
		.getc = uart0_getc,
		.putc = uart0_putc,
		.puts = uart0_puts,
		.flush = uart0_flush,
		.getraw = uart0_getraw,
		.handle_irq = uart0_handle_irq,
		.do_rx = uart0_do_rx,
	}
};

void uart_init(enum UART_TYPE type)
{
	cur_uart_type = type;
	uart_instance[cur_uart_type].init();
}

char uart_getc()
{
	if (uart_instance[cur_uart_type].getc)
		return uart_instance[cur_uart_type].getc();

	return 0;
}

void uart_putc(unsigned char c)
{
	if (uart_instance[cur_uart_type].putc)
		uart_instance[cur_uart_type].putc(c);
}

void uart_puts(const char *s)
{
	if (uart_instance[cur_uart_type].puts)
		uart_instance[cur_uart_type].puts(s);
}

void uart_flush()
{
	if (uart_instance[cur_uart_type].flush)
		uart_instance[cur_uart_type].flush();
}

char uart_getraw()
{
	if (uart_instance[cur_uart_type].getraw)
		return uart_instance[cur_uart_type].getraw();

	return 0;
}

void uart_handle_irq(enum UART_TYPE type)
{
	if (uart_instance[type].handle_irq)
		return uart_instance[type].handle_irq();
}

void uart_do_rx()
{
	if (uart_instance[cur_uart_type].do_rx)
		return uart_instance[cur_uart_type].do_rx();
}