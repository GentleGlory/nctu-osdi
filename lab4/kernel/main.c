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

static void task_1();
static void task_2();
static void task_3();
static void task_user_1();
static void task_user_2();
static void task_4();
static void task_5();

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


static void task_1()
{
	while (1) {
		
		printf("\r1...\n");
		delay(1000);
	}
}

static void task_2()
{
	while (1) {
		printf("\r2...\n");
		delay(1000);
	}
}

static void task_3()
{
	while (1) {
		printf("\r3...\n");
		delay(2000);
	}
}


static void task_user_1()
{
	while (1) {
		uprintf("\rUser task 1...\n");
		udelay(1000);
		system_call_exec(task_user_2);
	}
}

static void task_user_2()
{
	while (1) {
		uprintf("\rUser task 2...\n");
		udelay(1000);
		system_call_exec(task_user_1);
	}
}

static void task_4()
{
	printf("\r4...\n");
	do_exec(task_user_1);
}

static void task_5()
{
	printf("\r5...\n");
	do_exec(shell_main);
}

void main(void)
{
	uart_init(UART_TYPE_UART0);
	//uart_init(UART_TYPE_MINI_UART);
	
	timer_local_timer_init();
	print_board_info();

	if (fb_init() == 0)
		fb_draw_splash_image();

	uart_flush();

	task_init();
	scheduler_init();
	context_init();

	task_privilege_task_create(task_1, PRIORITY_LOW);
	task_privilege_task_create(task_2, PRIORITY_HIGH);
	task_privilege_task_create(task_3, PRIORITY_NORMAL);
	task_privilege_task_create(task_4, PRIORITY_NORMAL);
	task_privilege_task_create(task_5, PRIORITY_NORMAL);
	
	timer_core_timer_enable();
	irq_enable();
	
	while(1) {

		scheduler_do_schedule();

		asm("wfi");
	}
}