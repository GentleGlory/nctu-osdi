#ifndef _TIMER_H
#define _TIMER_H

#define CORE_TIMER_EXPIRE_PERIOD		0xfffffff

#define LOCAL_TIMER_CTRL_REG			0x40000034
#define LOCAL_TIMER_IRQ_CLR_REG			0x40000038

void core_timer_enable();
void core_timer_reload();

void local_timer_init();
void local_timer_reload();

#endif
