#include "lock.h"
#include "core.h"
#include "irq.h"


uint64_t lock_irq_save()
{
	uint64_t daif;

	// Read current DAIF value
	asm volatile(
		"mrs %0, daif\n"
		: "=r" (daif)
		:
		: "memory"
	);

	// Disable IRQ
	irq_disable();

	// Return only the I bit (bit 7) - 0 means IRQ enabled, 0x80 means IRQ disabled
	return daif & 0x80;
}

void lock_irq_restore(uint64_t irq_state)
{
	// irq_state 只包含 I bit (bit 7)
	// 如果之前是啟用 (0)，就啟用它
	// 如果之前是禁用 (0x80)，保持禁用狀態
	if (irq_state == 0) {
		irq_enable();
	}
}