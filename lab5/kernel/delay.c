#include "delay.h"
#include "scheduler.h"
#include "system_call.h"

void delay(uint64_t ms)
{
	scheduler_set_current_task_delay(ms);
	scheduler_do_schedule();
}

void udelay(uint64_t ms)
{
	system_call_delay(ms);
}