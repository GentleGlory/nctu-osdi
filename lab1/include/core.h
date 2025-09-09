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
typedef unsigned long uintptr_t;

static inline void writel(uintptr_t reg, u32 val)
{
	*(volatile u32*)(reg) = val;
}

static inline u32 readl(uintptr_t reg)
{
	return *(volatile u32*)(reg);
}

#endif