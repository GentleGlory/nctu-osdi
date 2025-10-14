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
#include "scheduler.h"

void print_board_info(void)
{
	uint32_t board_version, vc_mem_addr, vc_mem_size;
	
	board_version = mailbox_get_board_reversion();
	mailbox_get_vc_mem_info(&vc_mem_addr, &vc_mem_size);

	printf("\rBoard version:0x%x\n",board_version);
	printf("\rVC base address:0x%x, size:%u bytes\n",
			vc_mem_addr, vc_mem_size);
}

void do_exec(void(*func)())
{
	task_do_exec(func);
}


void task_1()
{
	while (1) {
		printf("\r1...\n");
		scheduler_do_schedule();
	}
}

void task_2()
{
	while (1) {
		printf("\r2...\n");
		scheduler_do_schedule();
	}
}

void task_3()
{
	while (1) {
		printf("\r3...\n");
		scheduler_do_schedule();
	}
}

void task_user()
{
	while (1) {
		printf("\rUser task...\n");
		scheduler_user_task_do_schedule();
	}
}

void task_4()
{
	printf("\r4...\n");
	do_exec(task_user);
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

	task_init();
	context_init();

	task_privilege_task_create(task_1);
	task_privilege_task_create(task_2);
	task_privilege_task_create(task_3);
	task_privilege_task_create(task_4);

	timer_core_timer_enable();
	irq_enable();
	
	//shell_main();
	while(1) {

		scheduler_do_schedule();
	}
}