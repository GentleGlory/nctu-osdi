#include "gpio.h"


void gpio_pin_sel_fn(int pin, enum GPF_FUNC func)
{
	u32 base, val;

	base = GPIO_GPFSEL_REG_BASE(pin / 10);
	val = readl(base);
	val &= GPIO_GPFSEL_CLEAR_MASK(pin);
	val |= GPIO_GPFSEL_FUNC(pin, func);
	writel(base, val);
}

void gpio_pin_set_pud(int pin, enum PULL_TYPE type)
{
	register unsigned int r;
	
	writel(GPIO_GPPUD_REG, type);
	r=150; while(r--){asm volatile("nop");}
	
	writel(GPIO_GPPUDCLK_REG_BASE(pin), GPIO_GPPUDCLK_ASSERT_LINE(pin));
	r=150; while(r--){asm volatile("nop");}
	
	writel(GPIO_GPPUDCLK_REG_BASE(pin), 0);
}