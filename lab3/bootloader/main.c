#include "uart.h"
#include "string.h"
#include "shell.h"

void main(void)
{
	uart_init(UART_TYPE_MINI_UART);

	printf("\rFrank bootloader init\n");

	uart_flush();

	shell_main();

	while(1) {

	}
}