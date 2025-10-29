#include "task.h"
#include "string.h"
#include "sys_regs.h"
#include "scheduler.h"
#include "exception.h"
#include "context.h"
#include "lock.h"
#include "delay.h"

struct task task_pool[TASK_POOL_SIZE] __attribute__((aligned(16)));
struct task *idle_task = &task_pool[0];

extern unsigned char __kernel_stack_start;
extern unsigned char __user_stack_start;


void task_init()
{
	unsigned char *kernel_sp = NULL;
	unsigned char *user_sp = NULL;
	unsigned char *kernel_reserved_sp = &__kernel_stack_start;
	unsigned char *user_reserved_sp = &__user_stack_start;

	// Initialize idle task

	idle_task->task_id = 0;
	idle_task->state = RUNNING;
	idle_task->need_reschedule = 1;
	memset(&idle_task->cpu_context, 0, sizeof(struct cpu_context));

	// Initialize task pool
	for (int i = 1; i < TASK_POOL_SIZE; i++) {
		task_pool[i].task_id = i;
		task_pool[i].state = EXIT;
		INIT_LIST_HEAD(&task_pool[i].list);

		kernel_sp = kernel_reserved_sp + KERNEL_STACK_SIZE * (i + 1) - 1;
		task_pool[i].reserved_kernel_sp = ((uint64_t) kernel_sp) & (~((uint64_t)0xf));

		user_sp = user_reserved_sp + USER_STACK_SIZE * (i + 1) - 1;
		task_pool[i].reserved_user_sp = ((uint64_t) user_sp) & (~((uint64_t)0xf));
	}
}

void task_privilege_task_create(void(*func)(),
		enum task_priority priority)
{
	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		if (task_pool[i].state == EXIT) {
			struct task *t = &task_pool[i];

			// Clear all CPU context registers
			memset(&t->cpu_context, 0, sizeof(struct cpu_context));

			// Set program counter to task function
			t->cpu_context.pc = (uint64_t) func;

			// Set up stack pointer (aligned to 16 bytes)
			t->cpu_context.sp = t->reserved_kernel_sp;
			
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

void task_exit(int status)
{
	uint64_t irq_state;
	struct task *cur = NULL;
	
	irq_state = lock_irq_save();
	
	cur = current;

	cur->state = ZOMBIE;
	cur->need_reschedule = 1;
	scheduler_remove_task_from_queue(cur);

	lock_irq_restore(irq_state);

	scheduler_do_schedule();
}

void task_zombie_reaper()
{	
	while (1) {
		
		for (int i = 0; i < TASK_POOL_SIZE; i++) {
			
			if (task_pool[i].state == ZOMBIE) {
				struct task *t = &task_pool[i];
				memset(&t->cpu_context, 0, sizeof(struct cpu_context));
				
				t->need_reschedule = 0;
				t->epoch = DEFAULT_EPOCH;
				t->priority = PRIORITY_LOW;
				t->runnable_task_parent = NULL;
				INIT_LIST_HEAD(&t->list);
				// Force state assignment to happen by using volatile cast
				*((volatile enum task_state *)&t->state) = EXIT;
			}
		}
		delay(5000);
	}
}

//Used by fork. not thread safe
void task_prepare_fork(struct pt_regs *src_pr_regs, uint64_t sp_address, uint64_t lr_address)
{
	uint64_t irq_state;
	struct task *dest = NULL;
	struct task *src = current;
	unsigned char *dest_sp = NULL;
	unsigned char *src_sp = NULL;
	struct pt_regs *dest_pt_regs = NULL;
	int64_t offset = 0;
	unsigned char *kernel_reserved_sp = &__kernel_stack_start;
	unsigned char *user_reserved_sp = &__user_stack_start;

	irq_state = lock_irq_save();

	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		if (task_pool[i].state == EXIT) {
			dest = &task_pool[i];
			break;
		}
	}

	if (dest == NULL) {
		printf("\rCannot find an usable task\n");
		src_pr_regs->regs[0] = (uint64_t) -1;
		goto Failed;
	}

	//clone cpu_context
	memcpy(&dest->cpu_context, &src->cpu_context, sizeof(struct cpu_context));

	dest->epoch = DEFAULT_EPOCH;
	dest->need_reschedule = 0;
	dest->priority = src->priority;
	dest->runnable_task_parent = NULL;
	dest->state = RUNNING;
	INIT_LIST_HEAD(&dest->list);

	//kernel sp
	dest_sp = kernel_reserved_sp + KERNEL_STACK_SIZE * dest->task_id;
	src_sp = kernel_reserved_sp + KERNEL_STACK_SIZE * src->task_id;
	memcpy(dest_sp, src_sp, KERNEL_STACK_SIZE);

	//User sp
	dest_sp = user_reserved_sp + USER_STACK_SIZE * dest->task_id;
	src_sp = user_reserved_sp + USER_STACK_SIZE * src->task_id;
	memcpy(dest_sp, src_sp, USER_STACK_SIZE);

	offset = (unsigned char *)src->reserved_kernel_sp - (unsigned char *)src_pr_regs;
	printf("\rpt_regs offset:%lld\n", offset);
	dest_pt_regs = (struct pt_regs *)((unsigned char *) dest->reserved_kernel_sp - offset);

	offset = (unsigned char *)src->reserved_user_sp - (unsigned char *)src_pr_regs->sp;
	printf("\ruser sp offset:%lld\n", offset);
	dest_pt_regs->sp = (uint64_t)((unsigned char *)dest->reserved_user_sp - offset);

	if (src_pr_regs->regs[29]) {
		offset = (unsigned char *)src->reserved_user_sp - (unsigned char *)src_pr_regs->regs[29];
		printf("\ruser fp offset:%lld\n", offset);
		dest_pt_regs->regs[29] = (uint64_t)((unsigned char *)dest->reserved_user_sp - offset);
	}

	dest->cpu_context.pc = lr_address;
	offset = (unsigned char *)src->reserved_kernel_sp - (unsigned char *)sp_address;
	printf("\rkernel sp offset:%lld\n", offset);
	dest->cpu_context.sp = (uint64_t)((unsigned char *)dest->reserved_kernel_sp - offset);

	if (src->cpu_context.fp) {
		offset = (unsigned char *)src->reserved_kernel_sp - (unsigned char *)src->cpu_context.fp;
		printf("\rkernel fp offset:%lld\n", offset);
		dest->cpu_context.fp = (uint64_t)((unsigned char *)dest->reserved_kernel_sp - offset);
	}

	scheduler_add_task_to_queue(dest, RUNNABLE_TASK_CURRENT);

	dest_pt_regs->regs[0] = 0;
	src_pr_regs->regs[0] = dest->task_id;
Failed:
	lock_irq_restore(irq_state);
}

int task_get_cur_task_id()
{
	struct task *cur = current;
	return cur->task_id;
}