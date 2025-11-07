#ifndef _TASK_H
#define _TASK_H

#ifndef __LINKER__
#include "core.h"
#include "list.h"
#include "exception.h"
#include "page.h"
#endif

#define TASK_POOL_SIZE	64

#define KERNEL_STACK_SIZE			0x1000

#define KERNEL_STACK_RESERVED_SIZE	(KERNEL_STACK_SIZE * TASK_POOL_SIZE)

#define USER_STACK_VIRT_ADDR		0x0000ffffffffe000

#define DEFAULT_EPOCH		10

#ifndef __LINKER__

enum task_state {
	RUNNING,
	ZOMBIE,
	EXIT,
};

enum task_priority {
	PRIORITY_LOW,
	PRIORITY_NORMAL,
	PRIORITY_HIGH,
	PRIORITY_MAX,
};

struct cpu_context{
	uint64_t	x19;
	uint64_t	x20;
	uint64_t	x21;
	uint64_t	x22;
	uint64_t	x23;
	uint64_t	x24;
	uint64_t	x25;
	uint64_t	x26;
	uint64_t	x27;
	uint64_t	x28;
	uint64_t	fp;
	uint64_t	sp;
	uint64_t	pc;
};

struct task {
	struct cpu_context	cpu_context;			// 104 bytes, offset 0
	int					need_reschedule;		// 4 bytes, offset 104
	enum task_priority	priority;				// 4 bytes, offset 108
	uint64_t			reserved_user_sp;		// 8 bytes, offset 112
	uint64_t			reserved_kernel_sp;		// 8 bytes, offset 120
	uint64_t			unblock_time;			// 8 bytes, offset 128
	struct list_head	*runnable_task_parent;	// 8 bytes, offset 136
	struct list_head	list;					// 16 bytes, offset 144
	int					task_id;				// 4 bytes, offset 160
	int					epoch;					// 4 bytes, offset 164
	enum task_state		state;					// 8 bytes, offset 168
	struct page			*user_pgd_page;
	// Total size: 176 bytes
} __attribute__((aligned(16)));

extern struct task task_pool[TASK_POOL_SIZE];

extern struct task *idle_task;

void task_init();

void task_privilege_task_create(void(*func)(),
		enum task_priority priority);

void task_do_exec(uint64_t binary_start, uint64_t binary_size,
		uint64_t virtual_addr_start);

void task_exit(int status);

void task_zombie_reaper();

void task_do_fork(struct pt_regs *pt_regs);

int task_get_cur_task_id();


#endif

#endif