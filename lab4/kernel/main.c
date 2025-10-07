#include "uart0.h"
#include "string.h"
#include "shell.h"
#include "mailbox.h"
#include "fb.h"
#include "uart.h"
#include "irq.h"
#include "timer.h"

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
	uart_init(UART_TYPE_UART0);
	
	local_timer_init();

	printf("\rFrank OS init\n");
	print_board_info();

	if (fb_init() == 0)
		fb_draw_splash_image();

	uart_flush();

	shell_main();

	while(1) {

	}
}