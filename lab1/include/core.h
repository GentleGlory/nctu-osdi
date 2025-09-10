#ifndef _CORE_H
#define _CORE_H

#define MMIO_BASE	0x3f000000

#define BITS_PER_LONG 64

#define UL(x) ((unsigned long)(x))

#define BIT(nr)		(UL(1) << (nr))

#define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & \
     (~0UL >> (BITS_PER_LONG - 1 - (h))))

typedef unsigned int u32;

static inline void writel(unsigned long long address, u32 val)
{
	//*(volatile u32*)(reg) = val;
	volatile u32* reg = (volatile u32*) address;
	(*reg) = val;
}

static inline u32 readl(unsigned long long address)
{
	//return *(volatile u32*)(reg);
	volatile u32* reg = (volatile u32*) address;
	return (*reg);
}

#endif