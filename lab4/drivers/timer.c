#include "timer.h"
#include "irq.h"
#include "string.h"
#include "uart.h"

static uint64_t core_timer_jiffies = 0;
//static uint64_t local_timer_jiffies = 0;
static uint64_t system_timer_jiffies = 0;

void timer_core_timer_enable()
{
	asm volatile (
		"mov x0, #1 \n"
		"msr cntp_ctl_el0, x0 \n"
		"ldr x0, =%0 \n"
		"msr cntp_tval_el0, x0 \n"
		"mov x0, #2 \n"
		"ldr x1, =%1 \n"
		"str x0, [x1] \n"
		:
		: "i" (CORE_TIMER_EXPIRE_PERIOD), "i" (CORE0_TIMER_IRQ_CTRL_REG)
		: "x0", "x1", "memory"
	);
}

void timer_core_timer_reload()
{
	asm volatile (
		"ldr x0, =%0 \n"
		"msr cntp_tval_el0, x0 \n"
		:
		: "i" (CORE_TIMER_EXPIRE_PERIOD)
		: "x0", "memory"
	);

	printf("\rCore timer jiffies:%llu\n", ++core_timer_jiffies);
}

void timer_local_timer_init()
{
	uint32_t flag = 0x30000000; // enable timer and interrupt.
	uint32_t reload = 12500000;

	writel(LOCAL_TIMER_CTRL_REG, flag | reload);
}

void timer_local_timer_reload()
{
	writel(LOCAL_TIMER_IRQ_CLR_REG, 0xc0000000); // clear interrupt and reload.
	//printf("\rLocal timer jiffies:%llu\n", ++local_timer_jiffies);

	uart_do_rx();
}

void timer_system_timer_init()
{
	uint32_t val = 0;
	val = readl(SYSTEM_TIMER_CLO_REG);
	writel(SYSTEM_TIMER_COMPARE1_REG, val + 2500000);
	irq_enable_arm_peri(ARM_PERI_IRQ_NUM_SYSTEM_TIMER_1);
}

void timer_system_timer_reload()
{
	unsigned int val = 0;
	val = readl(SYSTEM_TIMER_CLO_REG);
	writel(SYSTEM_TIMER_COMPARE1_REG, val + 2500000);
	writel(SYSTEM_TIMER_CS_REG, 0xf);

	printf("\rSystem timer jiffies:%llu\n", ++system_timer_jiffies);
}