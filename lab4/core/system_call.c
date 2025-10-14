#include "system_call.h"
#include "string.h"
#include "time.h"
#include "timer.h"
#include "exception.h"

#ifndef BOOTLOADE
#include "scheduler.h"
#endif

static void system_call_time_stamp_handler()
{
	struct rational r = {0, 0};
	
	r = time_get_time_tick();
	
	printf("\r[%lld.%lld]\n",r.num, r.den);
}

static void system_call_irq_test_handler()
{
	timer_core_timer_enable();
	timer_system_timer_init();
}

void system_call_run(uint32_t sys_call_num)
{
	// Execute SVC instruction with system call number
	// The system call number is passed in x8 register (Linux ARM64 standard)
	// SVC immediate value is always 0, actual syscall number is in x8
	asm volatile(
		"mov x8, %0\n"
		"svc #0\n"
		:
		: "r"(sys_call_num)
		: "x8"
	);
}

void system_call_user_task_do_schedule()
{
#ifndef BOOTLOADER	
	scheduler_do_schedule();
#endif	
}

void system_call_handler(uint64_t syscall_num, uint64_t esr, uint64_t elr)
{
	switch (syscall_num) {
		case SYS_CALL_TEST:
			int ec = EXC_ESR_EC(esr);
			int iss = esr & 0x1FFFFFF;

			printf("\rException return address 0x%x\n", elr);
			printf("\rException class (EC) 0x%x\n", ec);
			printf("\rInstruction specific syndrome (ISS) 0x%x\n", iss);
		break;
		case SYS_CALL_TIME_STAMP:
			system_call_time_stamp_handler();
		break;
		case SYS_CALL_IRQ_TEST:
			system_call_irq_test_handler();
		break;
		case SYS_CALL_SCHEDULE:
			system_call_user_task_do_schedule();
		break;
		default:
			printf("\rUnhandled system call num:%lld\n", syscall_num);
	}
}