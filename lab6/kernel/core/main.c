#include "uart0.h"
#include "string.h"
#include "mailbox.h"
#include "fb.h"
#include "uart.h"
#include "irq.h"
#include "timer.h"
#include "task.h"
#include "context.h"
#include "scheduler.h"
#include "delay.h"
#include "system_call.h"
#include "page.h"


// -----------below is kernel code-------------

void print_board_info(void)
{
	uint32_t board_version, vc_mem_addr, vc_mem_size;
	
	board_version = mailbox_get_board_reversion();
	mailbox_get_vc_mem_info(&vc_mem_addr, &vc_mem_size);

	printf("\rBoard version:0x%x\n",board_version);
	printf("\rVC base address:0x%x, size:%u bytes\n",
			vc_mem_addr, vc_mem_size);
}

void foo_1()
{
	struct task * cur = current;
	while(1) {
		printf("\rTask id: %d\n", cur->task_id);
		delay(1000);
	}
}

DEFINE_USER_PROGRAM_RUNNER(shell);
DEFINE_USER_PROGRAM_RUNNER(test_command1);
DEFINE_USER_PROGRAM_RUNNER(test_command2);
DEFINE_USER_PROGRAM_RUNNER(test_command3);

void main(void)
{
	uart_init(UART_TYPE_MINI_UART);
	page_init();
	uart_init(UART_TYPE_UART0);

	timer_local_timer_init();
	print_board_info();

	if (fb_init() == 0)
		fb_draw_splash_image();

	uart_flush();

	task_init();
	scheduler_init();
	context_init();

	//for(int i = 0; i < 3; ++i) { // N should > 2
	//	task_privilege_task_create(foo_1, PRIORITY_NORMAL);
	//}
	task_privilege_task_create(run_shell, PRIORITY_LOW);
	task_privilege_task_create(run_test_command1, PRIORITY_LOW);
	task_privilege_task_create(run_test_command2, PRIORITY_LOW);
	task_privilege_task_create(run_test_command3, PRIORITY_LOW);
	task_privilege_task_create(task_zombie_reaper, PRIORITY_LOW);

	timer_core_timer_enable();
	irq_enable();
	
	while(1) {

		scheduler_do_schedule();

		asm("wfi");
	}
}