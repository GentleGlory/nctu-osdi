#include "system_call.h"
#include "string.h"
#include "time.h"
#include "timer.h"
#include "exception.h"
#include "uart.h"
#include "task.h"
#include "context.h"
#include "delay.h"
#include "scheduler.h"
#include "mmu.h"

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
	scheduler_do_schedule();
}

static size_t system_call_uart_read_handler(char buf[], size_t size)
{	
	char temp[SYSTEM_CALL_UART_BUF_SIZE];
	size_t temp_size = size > SYSTEM_CALL_UART_BUF_SIZE ? SYSTEM_CALL_UART_BUF_SIZE : size;
	size_t ret = 0;
	struct task *cur = current;
	
	ret = uart_read(temp, temp_size);

	if (ret) {
		mmu_copy_to_user(cur->user_pgd_page, (uint64_t) buf, temp, ret);
	}

	return ret;
}

static size_t system_call_uart_write_handler(const char buf[], size_t size)
{
	char temp[SYSTEM_CALL_UART_BUF_SIZE];
	size_t temp_size = size > SYSTEM_CALL_UART_BUF_SIZE ? SYSTEM_CALL_UART_BUF_SIZE : size;
	size_t ret = 0;
	struct task *cur = current;
	
	mmu_copy_from_user(cur->user_pgd_page, temp, (uint64_t)buf, temp_size);
	
	ret = uart_write(temp, size);

	return ret;
}

static int system_call_exec_handler(struct pt_regs *pt_regs)
{
	//task_do_exec((void(*)())pt_regs->regs[0]);
	return 0;
}

static void system_call_fork_handler(struct pt_regs *pt_regs)
{
	task_do_fork(pt_regs);
}

static void system_call_exit_handler(int status)
{
	task_exit(status);
}

static void system_call_delay_handler(uint64_t ms)
{
	delay(ms);
}

static int system_call_get_task_id_handler()
{
	return task_get_cur_task_id();
}

static uint64_t system_call_remain_page_num_handler()
{
	return page_remain_page_num();
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
			system_call_fork_handler(pt_regs);
		break;
		case SYS_CALL_TASK_EXIT:
			system_call_exit_handler((int)pt_regs->regs[0]);
		break;
		case SYS_CALL_DELAY:
			system_call_delay_handler(pt_regs->regs[0]);
		break;
		case SYS_CALL_GET_TASK_ID:
			pt_regs->regs[0] = (uint64_t)system_call_get_task_id_handler();
		break;
		case SYS_CALL_GET_REMAIN_PAGE_NUM:
			pt_regs->regs[0] = (uint64_t)system_call_remain_page_num_handler();
		break;
		default:
			printf("\rUnhandled system call num:%lld\n", syscall_num);
	}	
}
