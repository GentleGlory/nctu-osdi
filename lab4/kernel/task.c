#include "task.h"
#include "string.h"
#include "sys_regs.h"
#include "scheduler.h"
#include "exception.h"
#include "context.h"

struct task task_pool[TASK_POOL_SIZE] __attribute__((aligned(16)));
struct task idle_task __attribute__((aligned(16)));

extern unsigned char __kernel_stack_start;
extern unsigned char __user_stack_start;


void task_init()
{
	// Initialize idle task
	idle_task.task_id = -1;
	idle_task.state = RUNNING;
	idle_task.need_reschedule = 1;
	memset(&idle_task.cpu_context, 0, sizeof(struct cpu_context));

	// Initialize task pool
	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		task_pool[i].task_id = i;
		task_pool[i].state = EXIT;
		INIT_LIST_HEAD(&task_pool[i].list);
	}
}

void task_privilege_task_create(void(*func)(),
		enum task_priority priority)
{
	unsigned char *kernel_sp = NULL;
	unsigned char *user_sp = NULL;
	unsigned char *kernel_reserved_sp = &__kernel_stack_start;
	unsigned char *user_reserved_sp = &__user_stack_start;

	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		if (task_pool[i].state == EXIT) {
			struct task *t = &task_pool[i];

			// Clear all CPU context registers
			memset(&t->cpu_context, 0, sizeof(struct cpu_context));

			// Set program counter to task function
			t->cpu_context.pc = (uint64_t) func;

			// Set up stack pointer (aligned to 16 bytes)
			kernel_sp = kernel_reserved_sp + KERNEL_STACK_SIZE * i - 1;
			t->cpu_context.sp = ((uint64_t) kernel_sp) & (~((uint64_t)0xf));
			t->reserved_kernel_sp = t->cpu_context.sp;

			user_sp = user_reserved_sp + USER_STACK_SIZE * i - 1;
			t->reserved_user_sp = ((uint64_t) user_sp) & (~((uint64_t)0xf));

			// Mark task as ready to run
			// IMPORTANT: Must set state explicitly to RUNNING (0) to prevent compiler
			// from assuming it's already 0 and optimizing away this assignment
			t->need_reschedule = 0;
			t->epoch = DEFAULT_EPOCH;
			t->priority = priority;
			t->runnable_task_parent = NULL;

			// Force state assignment to happen by using volatile cast
			*((volatile enum task_state *)&t->state) = RUNNING;

			scheduler_add_task_to_queue(t, RUNNABLE_TASK_CURRENT);
			break;
		}
	}
}

void task_do_exec(void(*func)())
{
	ret_to_user(func);
}