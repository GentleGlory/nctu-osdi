#include "system_call.h"

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

int system_call_get_task_id()
{
	return (int)system_call_run(SYS_CALL_GET_TASK_ID, 0, 0, 0, 0, 0, 0);
}

uint64_t system_call_remain_page_num()
{
	return (uint64_t)system_call_run(SYS_CALL_GET_REMAIN_PAGE_NUM, 0, 0, 0, 0, 0, 0);
}

static int64_t system_call_run(uint32_t sys_call_num,
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