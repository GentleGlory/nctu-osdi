#ifndef _IRQ_H
#define _IRQ_H

#include "core.h"

#define IRQ_BASE	(MMIO_BASE + 0x0000B000)

#define IRQ_BASIC_PENDING_REG		(IRQ_BASE + 0x200)
#define   IRQ_BASIC_PENDING_UART	BIT(19)
#define   IRQ_BASIC_PENDING_2	    BIT(9)
#define   IRQ_BASIC_PENDING_1	    BIT(8)

#define IRQ_PENDING_1_REG		(IRQ_BASE + 0x204)
#define IRQ_PENDING_2_REG		(IRQ_BASE + 0x208)

#define IRQ_1_ENABLE_REG		(IRQ_BASE + 0x210)
#define IRQ_2_ENABLE_REG		(IRQ_BASE + 0x214)

#define IRQ_BASIC_ENABLE_REG	(IRQ_BASE + 0x218)

#define IRQ_1_DISABLE_REG		(IRQ_BASE + 0x21C)
#define IRQ_2_DISABLE_REG		(IRQ_BASE + 0x220)
#define IRQ_BASIC_DISABLE_REG	(IRQ_BASE + 0x224)

#define CORE0_TIMER_IRQ_CTRL_REG		0x40000040
#define CORE0_IRQ_SRC_REG				0x40000060
#define   CORE0_IRQ_SRC_CNTPNSIRQ		BIT(1)
#define   CORE0_IRQ_SRC_LOCAL_TIMER		BIT(11)

#define ARM_IRQ_PERI_REG_BASE(num)		(IRQ_1_ENABLE_REG + (((num) / 32) * 0x4))
#define ARM_IRQ_PERI_IRQ_NUM(num)		((num) - ((num) / 32 ) * 32)

#define ARM_IRQ_PENDING_REG_BASE(num)	(IRQ_PENDING_1_REG + (((num) / 32) * 0x4))
#define ARM_IRQ_PENDING_STATUS(num)		BIT(((num) - ((num) / 32 ) * 32))

#define ARM_PERI_IRQ_NUM_SYSTEM_TIMER_1	1
#define ARM_PERI_IRQ_NUM_UART			57

void irq_enable();
void irq_disable();

void irq_handler();

void irq_enable_arm_peri(uint32_t irq_num);

#endif

