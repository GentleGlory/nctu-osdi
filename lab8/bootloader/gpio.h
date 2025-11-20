#ifndef _GPIO_H
#define _GPIO_H

#include "core.h"

#define GPIO_BASE			(MMIO_BASE + 0x200000)

#define GPIO_GPFSEL_REG_BASE(n)		(GPIO_BASE + (n) * 0x04)
#define GPIO_GPFSEL_CLEAR_MASK(i)		(~(0x7 << (((i) % 10) * 3 )))
#define GPIO_GPFSEL_FUNC(p, f)		((f) << (((p) % 10) * 3 ))

#define GPIO_GPPUD_REG					(GPIO_BASE + 0x94)
#define GPIO_GPPUDCLK_REG_BASE(n)		(GPIO_BASE + 0x98 + 4 * ((n) / 32))
#define   GPIO_GPPUDCLK_ASSERT_LINE(n)	( 1 << ((n) % 32))



enum PULL_TYPE {
	PULL_TYPE_UP = 2,
	PULL_TYPE_DOWN = 1,
	PULL_TYPE_NONE = 0,
};

enum GPF_FUNC {
	GPF_FUNC_INPUT = 0,
	GPF_FUNC_OUTPUT = 1,
	GPF_FUNC_0 = 4,
	GPF_FUNC_1 = 5,
	GPF_FUNC_2 = 6,
	GPF_FUNC_3 = 7,
	GPF_FUNC_4 = 3,
	GPF_FUNC_5 = 2,
	GPF_FUNC_MAX = 8,
};

void gpio_pin_sel_fn(int pin, enum GPF_FUNC func);
void gpio_pin_set_pud(int pin, enum PULL_TYPE type);

#endif