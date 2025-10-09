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
	struct cpu_context	cpu_context;

	int					task_id;
	enum task_state		state;
	int					need_reschedule;
	int					epoch;
	uint64_t			__padding;		// Padding to align list to 16 bytes
	struct list_head	list;
} __attribute__((aligned(16)));

extern struct task task_pool[TASK_POOL_SIZE];

extern struct task idle_task;

void task_init();

void task_privilege_task_create(void(*func)());

#endif

#endif