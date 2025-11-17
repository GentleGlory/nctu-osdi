#include "list.h"
#include "task.h"
#include "scheduler.h"
#include "context.h"
#include "string.h"
#include "system_call.h"
#include "irq.h"
#include "lock.h"
#include "kmalloc.h"

LIST_HEAD(delayed_task1);
LIST_HEAD(delayed_task2);

static struct list_head *cur_delayed_list = NULL;
static struct list_head *overflow_delayed_list = NULL;

static uint32_t ready_priorities_1 = 0;
static struct list_head *runnable_task_list_1 = NULL;

static uint32_t ready_priorities_2 = 0;
static struct list_head *runnable_task_list_2 = NULL;

static uint32_t *cur_ready_priorities = NULL;
static struct list_head *cur_runnable_task_list = NULL;

static uint32_t *next_ready_priorities = NULL;
static struct list_head *next_runnable_task_list = NULL;

static volatile uint64_t scheduler_tick_count = 0;
static volatile uint64_t scheduler_next_unblock_time = 0;

void scheduler_init()
{
	runnable_task_list_1 = kmalloc_alloc(sizeof(struct list_head) * PRIORITY_MAX);
	runnable_task_list_2 = kmalloc_alloc(sizeof(struct list_head) * PRIORITY_MAX);

	INIT_LIST_HEAD(&delayed_task1);
	INIT_LIST_HEAD(&delayed_task2);

	cur_delayed_list = &delayed_task1;
	overflow_delayed_list = &delayed_task2;

	for (int i = PRIORITY_LOW; i < PRIORITY_MAX; i++) {
		INIT_LIST_HEAD(&runnable_task_list_1[i]);
		INIT_LIST_HEAD(&runnable_task_list_2[i]);
	}

	cur_ready_priorities = &ready_priorities_1;
	next_ready_priorities = &ready_priorities_2;
	cur_runnable_task_list = runnable_task_list_1;
	next_runnable_task_list = runnable_task_list_2;

	scheduler_tick_count = 0;
	scheduler_next_unblock_time = UINT64_MAX;
}

static void schdeuler_reset_unblock_time()
{
	struct task *task = NULL;

	if (list_empty(cur_delayed_list)) {
		scheduler_next_unblock_time = UINT64_MAX;
	} else {
		task = list_first_entry(cur_delayed_list, struct task, list);
		scheduler_next_unblock_time = task->unblock_time;
	}
}

static void scheduler_update_tick_count()
{
	const uint64_t tick_count = scheduler_tick_count + 1;
	scheduler_tick_count = tick_count;
	struct list_head *temp;
	struct task *task = NULL;
	uint64_t irq_state;
	
	irq_state = lock_irq_save();
	//overflow
	if (tick_count == 0) {
		temp = cur_delayed_list;
		cur_delayed_list = overflow_delayed_list;
		overflow_delayed_list = temp;
		schdeuler_reset_unblock_time();
	}

	if (tick_count >= scheduler_next_unblock_time) {

		for (;;) {
			
			if (list_empty(cur_delayed_list)) {
				scheduler_next_unblock_time = UINT64_MAX;
				break;
			} else {
				task = list_first_entry(cur_delayed_list, struct task, list);

				if (tick_count < task->unblock_time) {
					scheduler_next_unblock_time = task->unblock_time;
					break;
				}

				list_del(&task->list);
				scheduler_add_task_to_queue(task, RUNNABLE_TASK_ORIGINAL);
			}
		}
	}

	lock_irq_restore(irq_state);
}

static void scheduler_update_task_epoch()
{
	struct task *cur = current;
	if (cur != idle_task) {
		cur->epoch --;
		
		if (cur->epoch <= 0) {
			//printf("\rtask id:%d, need reschedule.\n", cur->task_id);
			cur->need_reschedule = 1;
		}
	}
}

static void scheduler_switch_runnable_task_list()
{
	struct list_head *temp1 = cur_runnable_task_list;
	uint32_t *temp2 = cur_ready_priorities;

	cur_runnable_task_list = next_runnable_task_list;
	cur_ready_priorities = next_ready_priorities;

	next_runnable_task_list = temp1;
	next_ready_priorities = temp2;	
}

static struct task *scheduler_get_next_task()
{
	struct task *ret = idle_task;
	uint32_t top_priority = 0;
	uint64_t irq_state;

	irq_state = lock_irq_save();

	if ((*cur_ready_priorities) != 0) {

		SCHEDULER_GET_HIGHEST_PRIORITY(top_priority, (*cur_ready_priorities));
		
		ret = list_first_entry(&cur_runnable_task_list[top_priority], struct task, list);
		scheduler_remove_task_from_queue(ret);
		//add to tail
		scheduler_add_task_to_queue(ret, RUNNABLE_TASK_NEXT);
	} else {
		scheduler_switch_runnable_task_list();
	}

	lock_irq_restore(irq_state);

	return ret;
}

void scheduler_do_schedule()
{
	struct task *next = NULL;
	struct task *cur = current;
	
	if (cur->need_reschedule) {
		
		next = scheduler_get_next_task();
		if (next != cur) {
			uint64_t irq_state;

			if (cur != idle_task) {
				cur->need_reschedule = 0;

				if (cur->epoch <= 0)
					cur->epoch = DEFAULT_EPOCH;
			}
			
			irq_state = lock_irq_save();
			context_switch(next, irq_state);
			lock_irq_restore(irq_state);
		}
	}
}

void scheduler_process()
{
	scheduler_update_task_epoch();
	scheduler_update_tick_count();
}

void scheduler_add_task_to_queue(struct task * task, enum RUNNABLE_TASK_TYPE type)
{
	struct list_head *runnable_task_list = cur_runnable_task_list;
	uint32_t *ready_priorities = cur_ready_priorities;
	
	if (type == RUNNABLE_TASK_NEXT) {
		runnable_task_list = next_runnable_task_list;
		ready_priorities = next_ready_priorities;
	} else if (type == RUNNABLE_TASK_ORIGINAL) {
		runnable_task_list = task->runnable_task_parent;
		ready_priorities = task->runnable_task_parent == runnable_task_list_1 ?
				&ready_priorities_1 : &ready_priorities_2;
	}

	list_add_tail(&task->list, &runnable_task_list[task->priority]);
	SCHEDULER_RECORD_READY_PRIORITY(task->priority, (*ready_priorities));

	task->runnable_task_parent = runnable_task_list;
}

void scheduler_remove_task_from_queue(struct task * task)
{
	uint32_t *ready_priorities = 
		task->runnable_task_parent == runnable_task_list_1 ?
				&ready_priorities_1 : &ready_priorities_2;
	
	list_del(&task->list);
	if (list_empty(&task->runnable_task_parent[task->priority])) {
		SCHEDULER_RESET_READY_PRIORITY(task->priority, (*ready_priorities));
	}
}

static void scheduler_insert_task_to_delaylist(
	struct task *cur, struct list_head *delay_list)
{
	struct task *task_entry;
	struct list_head *pos;

	// Find the correct position to insert based on unblock_time
	// List is sorted in ascending order by unblock_time
	list_for_each(pos, delay_list) {
		task_entry = list_entry(pos, struct task, list);
		
		if (cur->unblock_time < task_entry->unblock_time) {
			// Insert before this task
			list_add(&cur->list, pos->prev);
			return;
		}
	}
	// If we get here, insert at the end (tail)
	list_add_tail(&cur->list, delay_list);
}

void scheduler_set_current_task_delay(uint64_t ms)
{
	struct task *cur = current;
	uint64_t time_to_wake;
	const uint64_t cur_tick_cnt = scheduler_tick_count;
	uint64_t irq_state;

	irq_state = lock_irq_save();
	
	scheduler_remove_task_from_queue(cur);

	time_to_wake = cur_tick_cnt + ms;

	cur->unblock_time = time_to_wake;
	cur->need_reschedule = 1;

	//over flow
	if (time_to_wake < cur_tick_cnt) {
		scheduler_insert_task_to_delaylist(cur, overflow_delayed_list);
	} else {
		scheduler_insert_task_to_delaylist(cur, cur_delayed_list);

		if (time_to_wake < scheduler_next_unblock_time) {
			scheduler_next_unblock_time = time_to_wake;
		}
	}

	lock_irq_restore(irq_state);
}
