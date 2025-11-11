#include "user_lib.h"
#include "system_call.h"

void print_timestamp()
{
	system_call_print_timestamp();
}

void irq_test()
{
	syetem_call_irq_test();
}

size_t uart_read(char buf[], size_t size)
{
	return system_call_uart_read(buf, size);
}

size_t uart_write(const char buf[], size_t size)
{
	return system_call_uart_write(buf, size);	
}

int fork()
{
	return system_call_fork();
}

void exit(int status)
{
	system_call_exit(status);
}

void delay(uint64_t ms)
{
	system_call_delay(ms);
}

int get_task_id()
{
	return system_call_get_task_id();
}

void exc_test()
{
	system_call_test();
}

uint64_t remain_page_num()
{
	return system_call_remain_page_num();
}