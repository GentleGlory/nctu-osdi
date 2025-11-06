#include "uart0.h"
#include "string.h"
#include "shell.h"

void main(void)
{
	uart0_init();

	printf("\rFrank bootloader init\n");

	uart0_flush();

	shell_main();

	while(1) {

	}
}