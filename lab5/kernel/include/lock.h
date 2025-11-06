#ifndef _LOCK_H
#define _LOCK_H

#include "core.h"

uint64_t lock_irq_save();
void lock_irq_restore(uint64_t irq_state);


#endif