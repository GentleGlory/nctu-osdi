#include "irq.h"
#include "timer.h"
#include "string.h"

void irq_enable()
{
	asm volatile("msr daifclr, #2" ::: "memory");
}

void irq_disable()
{
	asm volatile("msr daifset, #2" ::: "memory");
}

static void irq_core_timer_handler()
{
	core_timer_reload();
}

static void irq_local_timer_handler()
{
	local_timer_reload();
}

void irq_handler()
{
	//uint32_t basis_pending = readl(IRQ_BASIC_PENDING_REG);
	uint32_t core0_irq_src = readl(CORE0_IRQ_SRC_REG);
	
	if (core0_irq_src & CORE0_IRQ_SRC_CNTPNSIRQ) {
		irq_core_timer_handler();
	} else if (core0_irq_src & CORE0_IRQ_SRC_LOCAL_TIMER) {
		irq_local_timer_handler();
	} else {
		printf("\rUnhandled irq\n");
	}
}
