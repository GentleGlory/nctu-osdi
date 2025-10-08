#include "uart0.h"
#include "string.h"
#include "shell.h"
#include "mailbox.h"
#include "fb.h"
#include "uart.h"
#include "irq.h"
#include "timer.h"
#include "task.h"
#include "context.h"

void print_board_info(void)
{
	uint32_t board_version, vc_mem_addr, vc_mem_size;
	
	board_version = mailbox_get_board_reversion();
	mailbox_get_vc_mem_info(&vc_mem_addr, &vc_mem_size);

	printf("\rBoard version:0x%x\n",board_version);
	printf("\rVC base address:0x%x, size:%u bytes\n",
			vc_mem_addr, vc_mem_size);
}

void task_1()
{
	while(1) {
		printf("\r1...\n");
		context_switch(&task_pool[1]);
	}	
}

void task_2()
{
	while(1) {
		printf("\r2...\n");
		context_switch(&task_pool[0]);
	}
}

void main(void)
{
	//uart_init(UART_TYPE_UART0);
	uart_init(UART_TYPE_MINI_UART);
	
	//local_timer_init();
	print_board_info();

	if (fb_init() == 0)
		fb_draw_splash_image();

	uart_flush();

	init_task();

	privilege_task_create(task_1);
	privilege_task_create(task_2);
	
	set_current(&idle_task);
	context_switch(&task_pool[0]);
	
	//shell_main();
	while(1) {

	}
}