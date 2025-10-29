#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "task.h"

extern struct task *get_current();
extern void set_current(struct task *task);
extern void switch_to(struct task *prev, struct task *next, uint64_t irq_state);

#define current get_current();


void context_init();
void context_switch(struct task *next, uint64_t irq_state);

#endif
