#ifndef _CORE_H
#define _CORE_H

#define MMIO_BASE	0x3f000000

#define BITS_PER_LONG 64

#define UL(x) ((unsigned long)(x))

#define BIT(nr)		(UL(1) << (nr))

#define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & \
     (~0UL >> (BITS_PER_LONG - 1 - (h))))

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

#define INT64_MAX	(9223372036854775807LL)
#define INT64_MIN	(-9223372036854775807LL - 1)

#ifndef NULL
#define NULL ((void*)0)
#endif

static inline void writel(uint64_t address, uint32_t val)
{
	//*(volatile u32*)(reg) = val;
	volatile uint32_t* reg = (volatile uint32_t*) address;
	(*reg) = val;
}

static inline uint32_t readl(unsigned long long address)
{
	//return *(volatile u32*)(reg);
	volatile uint32_t* reg = (volatile uint32_t*) address;
	return (*reg);
}

#define read_sys_reg(reg) ({ \
	uint64_t __val; \
	asm volatile("mrs %0, " #reg : "=r" (__val)); \
	__val; \
})

#endif