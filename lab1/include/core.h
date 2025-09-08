#ifndef _CORE_H
#define _CORE_H

#include <stdint.h>

#define MMIO_BASE	0x3f000000

typedef uint32_t u32;

static inline void writel(u32 reg, u32 val)
{
	*(volatile u32*)(reg) = val;
}

static inline u32 readl(u32 reg) 
{
	return *(volatile u32*)(reg);
}

#endif