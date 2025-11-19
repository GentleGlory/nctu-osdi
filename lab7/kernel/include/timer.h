#ifndef _TIMER_H
#define _TIMER_H

//19200 1ms
#define CORE_TIMER_EXPIRE_PERIOD		0x4B00

#define LOCAL_TIMER_CTRL_REG			0x40000034
#define LOCAL_TIMER_IRQ_CLR_REG			0x40000038

#define SYSTEM_TIMER_CS_REG 		(MMIO_BASE + 0x3000)
#define SYSTEM_TIMER_COMPARE1_REG 	(MMIO_BASE + 0x3010)
#define SYSTEM_TIMER_CLO_REG 		(MMIO_BASE + 0x3004)


void timer_core_timer_enable();
void timer_core_timer_reload();

void timer_local_timer_init();
void timer_local_timer_reload();

void timer_system_timer_init();
void timer_system_timer_reload();

#endif
