#ifndef _CORE_H
#define _CORE_H

#ifndef __ASM__

#include "uapi/type.h"

#define MMIO_BASE	0x3f000000

#define BITS_PER_LONG 64

#define UL(x) ((unsigned long)(x))

#define BIT(nr)		(UL(1) << (nr))

#define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & \
     (~0UL >> (BITS_PER_LONG - 1 - (h))))


#define offsetof(type, member)  __builtin_offsetof(type, member)

#define container_of(ptr, type, member) ({						\
	const typeof(((type *)0)->member) *__mptr = (ptr);			\
	(type *)((char *)__mptr - offsetof(type, member)); })

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


#define assert(cond) \
	do { \
		if (!(cond)) { \
			printf("\rAssertion failed: %s, file %s, line %d\n", \
				#cond, __FILE__, __LINE__); \
				while(1); \
		} \
	} while (0)

#endif

#endif