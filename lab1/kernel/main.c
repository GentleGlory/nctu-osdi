#include "mini_uart.h"
#include "string.h"
#include "shell.h"




void main(void)
{
	mini_uart_init();

	printf("\rFrank OS init\n");

	mini_uart_flush();

	shell_main();

	while(1) {

	}
}