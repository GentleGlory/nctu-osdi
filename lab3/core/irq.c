#include "irq.h"
#include "timer.h"
#include "string.h"
#include "uart.h"

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

static void irq_system_timer_1_handler()
{
	system_timer_reload();
}

static void irq_uart0_handler()
{
	uart_handle_irq(UART_TYPE_UART0);
}

static void irq_pending_1_handler()
{
	uint32_t pending_1 = readl(IRQ_PENDING_1_REG);

	if (pending_1 & ARM_IRQ_PENDING_STATUS(ARM_PERI_IRQ_NUM_SYSTEM_TIMER_1)) {
		irq_system_timer_1_handler();
	} else {
		printf("\rrUnhandled irq pending_1\n");
	}
}

void irq_handler()
{
	uint32_t basic_pending = readl(IRQ_BASIC_PENDING_REG);
	uint32_t core0_irq_src = readl(CORE0_IRQ_SRC_REG);
	
	if (core0_irq_src & CORE0_IRQ_SRC_CNTPNSIRQ) {
		irq_core_timer_handler();
	} else if (core0_irq_src & CORE0_IRQ_SRC_LOCAL_TIMER) {
		irq_local_timer_handler();
	} else if (basic_pending & IRQ_BASIC_PENDING_UART) {
		irq_uart0_handler();
	} else if (basic_pending & IRQ_BASIC_PENDING_1) {
		irq_pending_1_handler();
	} else {
		printf("\rUnhandled irq\n");
	}
}

void irq_enable_arm_peri(uint32_t irq_num)
{
	uint32_t base = ARM_IRQ_PERI_REG_BASE(irq_num);
	uint32_t val = readl(base) | BIT((ARM_IRQ_PERI_IRQ_NUM(irq_num)));
	writel(base, val);
}
