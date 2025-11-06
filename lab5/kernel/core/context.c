#include "context.h"

void context_init()
{
	set_current(idle_task);
}

void context_switch(struct task *next, uint64_t irq_state)
{
	struct task *prev = current;
	switch_to(prev, next, irq_state);
}
