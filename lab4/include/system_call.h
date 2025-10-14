#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

#include "core.h"

#define SYS_CALL_TEST			0
#define SYS_CALL_TIME_STAMP		1
#define SYS_CALL_IRQ_TEST		2
#define SYS_CALL_SCHEDULE		3

void system_call_run(uint32_t sys_call_num);
void system_call_handler(uint64_t syscall_num, uint64_t esr, uint64_t elr);
#endif