#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "task.h"

enum RUNNABLE_TASK_TYPE{
	RUNNABLE_TASK_CURRENT,
	RUNNABLE_TASK_NEXT,
	RUNNABLE_TASK_ORIGINAL,
};


#define SCHEDULER_RECORD_READY_PRIORITY( priority, ready_priorities ) ( ( ready_priorities ) |= ( 1UL << ( priority ) ) )
#define SCHEDULER_RESET_READY_PRIORITY( priority, ready_priorities ) ( ( ready_priorities ) &= ~( 1UL << ( priority ) ) )
#define SCHEDULER_GET_HIGHEST_PRIORITY( top_priority, ready_priorities ) ( top_priority = ( 31UL - ( uint32_t ) __builtin_clz( ( ready_priorities ) ) ) )

void scheduler_init();
void scheduler_process();
void scheduler_do_schedule();
void scheduler_remove_task_from_queue(struct task * task);
void scheduler_add_task_to_queue(struct task * task, enum RUNNABLE_TASK_TYPE type);

void scheduler_set_current_task_delay(uint64_t ms);

#endif