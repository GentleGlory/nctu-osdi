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
#include "delay.h"
#include "system_call.h"
#include "page.h"

void foo_2(){
	int tmp = 5;
	uprintf("\rTask %d after exec, tmp address 0x%x, tmp value %d\n", system_call_get_task_id(), &tmp, tmp);
	system_call_exit(0);
}

void test() {
	int cnt = 1;
	if (system_call_fork() == 0) {
		system_call_fork();
		udelay(100);
		system_call_fork();
		while(cnt < 10) {
			uprintf("\rTask id: %d, cnt: %d\n", system_call_get_task_id(), cnt);
			udelay(1000);
			++cnt;
		}
		system_call_exit(0);
		uprintf("\rShould not be printed\n");
	} else {
		uprintf("\rTask %d before exec, cnt address 0x%x, cnt value %d\n", system_call_get_task_id(), &cnt, cnt);
		system_call_exec(foo_2);
	}
}

// -----------above is user code-------------
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

static void do_exec(void(*func)())
{
	task_do_exec(func);
}

void foo_1()
{
	struct task * cur = current;
	while(1) {
		printf("\rTask id: %d\n", cur->task_id);
		delay(1000);
	}
}

void user_test()
{
	do_exec(test);
}

void run_shell() 
{
	do_exec(shell_main);
}

void main(void)
{
	//uart_init(UART_TYPE_UART0);
	uart_init(UART_TYPE_MINI_UART);
	
	page_init();

	timer_local_timer_init();
	print_board_info();

	if (fb_init() == 0)
		fb_draw_splash_image();

	uart_flush();

	task_init();
	scheduler_init();
	context_init();

	for(int i = 0; i < 3; ++i) { // N should > 2
		task_privilege_task_create(foo_1, PRIORITY_NORMAL);
	}
	//task_privilege_task_create(user_test, PRIORITY_NORMAL);
	//task_privilege_task_create(run_shell, PRIORITY_LOW);
	task_privilege_task_create(task_zombie_reaper, PRIORITY_LOW);

	timer_core_timer_enable();
	irq_enable();
	
	while(1) {

		scheduler_do_schedule();

		asm("wfi");
	}
}