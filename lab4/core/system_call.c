#include "system_call.h"
#include "string.h"
#include "time.h"
#include "timer.h"
#include "exception.h"
#include "uart.h"
#include "task.h"
#include "context.h"
#include "delay.h"

#ifndef BOOTLOADE
#include "scheduler.h"
#endif

static int64_t system_call_run(uint32_t sys_call_num,
	uint64_t arg1, uint64_t arg2, uint64_t arg3,
	uint64_t arg4, uint64_t arg5, uint64_t arg6);

//System Call
void system_call_user_task_do_schedule()
{
	system_call_run(SYS_CALL_SCHEDULE, 0, 0, 0, 0, 0, 0);
}

void system_call_print_timestamp()
{
	system_call_run(SYS_CALL_PRINT_TIME_STAMP, 0, 0, 0, 0, 0, 0);
}

void system_call_test()
{
	system_call_run(SYS_CALL_TEST, 0, 0, 0, 0, 0, 0);
}

void syetem_call_irq_test()
{
	system_call_run(SYS_CALL_IRQ_TEST, 0, 0, 0, 0, 0, 0);
}

size_t system_call_uart_read(char buf[], size_t size)
{
	return (size_t) system_call_run(SYS_CALL_UART_READ,
		(uint64_t) buf, (uint64_t)size, 0, 0, 0, 0);
}

size_t system_call_uart_write(const char buf[], size_t size)
{
	return (size_t) system_call_run(SYS_CALL_UART_WRITE,
		(uint64_t) buf, (uint64_t)size, 0, 0, 0, 0);
}

int system_call_exec(void(*func)())
{
	return (int) system_call_run(SYS_CALL_TASK_EXEC,
		(uint64_t) func, 0, 0, 0, 0, 0);
}

int system_call_fork()
{
	return (int) system_call_run(SYS_CALL_TASK_FORK, 
		0, 0, 0, 0, 0, 0);
}

void system_call_exit(int status)
{
	system_call_run(SYS_CALL_TASK_EXIT, 
		(uint64_t)status, 0, 0, 0, 0, 0);
}

void system_call_delay(uint64_t ms)
{
	system_call_run(SYS_CALL_DELAY, ms, 0, 0, 0, 0, 0);
}

//Handler
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

static void system_call_user_task_do_schedule_handler()
{
#ifndef BOOTLOADER	
	scheduler_do_schedule();
#endif	
}

static size_t system_call_uart_read_handler(char buf[], size_t size)
{
	return uart_read(buf, size);
}

static size_t system_call_uart_write_handler(const char buf[], size_t size)
{
	return uart_write(buf, size);
}

static int system_call_exec_handler(struct pt_regs *pt_regs)
{
#ifndef BOOTLOADER
	struct task *cur = current;

	pt_regs->elr = pt_regs->regs[0];
	pt_regs->sp = cur->reserved_user_sp;
#endif
	return 0;
}

static int system_call_fork_handler()
{
	return 0;
}

static void system_call_exit_handler(int status)
{
	
}

static void system_call_delay_handler(uint64_t ms)
{
#ifndef BOOTLOADER
	delay(ms);
#endif
}

int64_t system_call_run(uint32_t sys_call_num,
	uint64_t arg1, uint64_t arg2, uint64_t arg3,
	uint64_t arg4, uint64_t arg5, uint64_t arg6)
{
	int64_t ret = -1;
	// Execute SVC instruction with system call number
	// The system call number is passed in x8 register (Linux ARM64 standard)
	// SVC immediate value is always 0, actual syscall number is in x8
	asm volatile(
		"mov x0, %2\n"
		"mov x1, %3\n"
		"mov x2, %4\n"
		"mov x3, %5\n"
		"mov x4, %6\n"
		"mov x5, %7\n"
		"mov x8, %1\n"
		"svc #0\n"
		"mov %0, x0\n"
		: "=r"(ret)
		: "r"(sys_call_num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
		: "x0", "x1", "x2", "x3", "x4", "x5", "x8"
	);

	return ret;
}

void system_call_exc_handler(struct pt_regs *pt_regs)
{
	uint64_t syscall_num = pt_regs->regs[8];
	
	switch (syscall_num) {
		case SYS_CALL_TEST:
			uint64_t esr = read_sys_reg(esr_el1);
			uint64_t elr = read_sys_reg(elr_el1);

			int ec = EXC_ESR_EC(esr);
			int iss = esr & 0x1FFFFFF;

			printf("\rException return address 0x%x\n", elr);
			printf("\rException class (EC) 0x%x\n", ec);
			printf("\rInstruction specific syndrome (ISS) 0x%x\n", iss);
		break;
		case SYS_CALL_PRINT_TIME_STAMP:
			system_call_time_stamp_handler();
		break;
		case SYS_CALL_IRQ_TEST:
			system_call_irq_test_handler();
		break;
		case SYS_CALL_SCHEDULE:
			system_call_user_task_do_schedule_handler();
		break;
		case SYS_CALL_UART_READ:
			pt_regs->regs[0] = (uint64_t)system_call_uart_read_handler((char *) pt_regs->regs[0],
				(size_t)pt_regs->regs[1]);
		break;
		case SYS_CALL_UART_WRITE:
			pt_regs->regs[0] = (uint64_t)system_call_uart_write_handler((const char *) pt_regs->regs[0],
				(size_t)pt_regs->regs[1]);
		break;
		case SYS_CALL_TASK_EXEC:
			pt_regs->regs[0] = (uint64_t)system_call_exec_handler(pt_regs);
		break;
		case SYS_CALL_TASK_FORK:
			pt_regs->regs[0] = (uint64_t)system_call_fork_handler();
		break;
		case SYS_CALL_TASK_EXIT:
			system_call_exit_handler((int)pt_regs->regs[0]);
		break;
		case SYS_CALL_DELAY:
			system_call_delay_handler(pt_regs->regs[0]);
		break;
		default:
			printf("\rUnhandled system call num:%lld\n", syscall_num);
	}	
}