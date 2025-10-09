#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "task.h"

void scheduler_update_task_epoch();
void scheduler_do_schedule();
void scheduler_add_task_to_queue(struct task * task);

#endif