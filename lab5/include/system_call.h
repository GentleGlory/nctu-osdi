#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

#include "core.h"
#include "exception.h"

#define SYS_CALL_TEST				0
#define SYS_CALL_PRINT_TIME_STAMP	1
#define SYS_CALL_IRQ_TEST			2
#define SYS_CALL_SCHEDULE			3
#define SYS_CALL_UART_READ			4
#define SYS_CALL_UART_WRITE			5
#define SYS_CALL_TASK_EXEC			6
#define SYS_CALL_TASK_FORK			7
#define SYS_CALL_TASK_EXIT			8
#define SYS_CALL_DELAY				9
#define SYS_CALL_GET_TASK_ID		10


void system_call_exc_handler(struct pt_regs *pt_regs);

void system_call_test();
void system_call_print_timestamp();
void syetem_call_irq_test();
void system_call_user_task_do_schedule();
size_t system_call_uart_read(char buf[], size_t size);
size_t system_call_uart_write(const char buf[], size_t size);
int system_call_exec(void(*func)());
int system_call_fork();
void system_call_exit(int status);
void system_call_delay(uint64_t ms);
int system_call_get_task_id();



#endif