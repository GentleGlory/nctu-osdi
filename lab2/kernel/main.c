#include "mini_uart.h"
#include "string.h"
#include "shell.h"
#include "mailbox.h"

void print_board_info(void)
{
	uint32_t board_version, vc_mem_addr, vc_mem_size;
	
	board_version = mailbox_get_board_reversion();
	mailbox_get_vc_mem_info(&vc_mem_addr, &vc_mem_size);

	printf("\rBoard version:0x%x\n",board_version);
	printf("\rVC base address:0x%x, size:%u bytes\n",
			vc_mem_addr, vc_mem_size);
}


void main(void)
{
	mini_uart_init();

	printf("\rFrank OS init\n");
	print_board_info();

	mini_uart_flush();

	shell_main();

	while(1) {

	}
}