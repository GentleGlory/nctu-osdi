#include "list.h"
#include "task.h"
#include "scheduler.h"
#include "context.h"
#include "string.h"

LIST_HEAD(runnable_task_list);

void scheduler_do_schedule()
{
	struct task *next = &idle_task;
	struct task *cur = current;

	if (cur->need_reschedule) {
		
		if (!list_empty(&runnable_task_list)) {
			next = list_first_entry(&runnable_task_list, struct task, list);
			list_del(&next->list);
		}
		
		if (next != cur) {
			if (cur != &idle_task) {
				cur->need_reschedule = 0;
				cur->epoch = DEFAULT_EPOCH;
				list_add_tail(&cur->list, &runnable_task_list);
			}
			
			context_switch(next);
		}
	}	
}

void scheduler_add_task_to_queue(struct task * task)
{
	list_add_tail(&task->list, &runnable_task_list);
}

void scheduler_update_task_epoch()
{
	struct task *cur = current;
	if (cur != &idle_task) {
		cur->epoch --;

		if (cur->epoch <= 0) {
			printf("\rtask id:%d, need reschedule.\n", cur->task_id);
			cur->need_reschedule = 1;
		}
	}
}