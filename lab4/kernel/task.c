#include "task.h"
#include "string.h"
#include "sys_regs.h"

struct task task_pool[TASK_POOL_SIZE];
struct task idle_task;

extern unsigned char __kernel_stack_start;
extern unsigned char __user_stack_start;


void init_task()
{
	// Initialize idle task
	idle_task.task_id = -1;
	idle_task.state = RUNNING;
	memset(&idle_task.cpu_context, 0, sizeof(struct cpu_context));

	// Initialize task pool
	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		task_pool[i].task_id = i;
		task_pool[i].state = EXIT;
	}
}

void privilege_task_create(void(*func)())
{
	unsigned char *sp = NULL;
	unsigned char *kernel_reserved_sp = &__kernel_stack_start;

	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		if (task_pool[i].state == EXIT) {
			// Clear all CPU context registers
			memset(&task_pool[i].cpu_context, 0, sizeof(struct cpu_context));

			// Set program counter to task function
			task_pool[i].cpu_context.pc = (uint64_t) func;

			// Set up stack pointer (aligned to 16 bytes)
			sp = kernel_reserved_sp + KERNEL_STACK_SIZE * i - 1;
			task_pool[i].cpu_context.sp = ((uint64_t) sp) & (~((uint64_t)0xf));

			// Mark task as ready to run
			task_pool[i].state = RUNNING;
			break;
		}
	}
}