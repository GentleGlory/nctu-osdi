#ifndef _TASK_H
#define _TASK_H

#ifndef __LINKER__
#include "core.h"
#include "list.h"
#endif

#define TASK_POOL_SIZE	64

#define KERNEL_STACK_SIZE			0x1000

#define KERNEL_STACK_RESERVED_SIZE	(KERNEL_STACK_SIZE * TASK_POOL_SIZE)

#define USER_STACK_SIZE			0x1000
#define USER_STACK_RESERVED_SIZE	(USER_STACK_SIZE * TASK_POOL_SIZE)

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
	uint64_t			reserved_user_sp;		// 8 bytes, offset 104
	struct list_head	list;					// 16 bytes, offset 112
	uint64_t			unblock_time;			// 8 bytes, offset 128
	int					task_id;				// 4 bytes, offset 132
	enum task_state		state;					// 4 bytes, offset 136
	int					need_reschedule;		// 4 bytes, offset 140
	int					epoch;					// 4 bytes, offset 144
	enum task_priority	priority;				// 4 bytes, offset 148
	struct list_head	*runnable_task_parent;	// 8 bytes, offset 152
	// Total size: 160 bytes
} __attribute__((aligned(16)));

extern struct task task_pool[TASK_POOL_SIZE];

extern struct task idle_task;

void task_init();

void task_privilege_task_create(void(*func)(),
		enum task_priority priority);

void task_do_exec(void(*func)());

#endif

#endif