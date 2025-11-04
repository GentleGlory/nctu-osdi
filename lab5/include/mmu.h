#ifndef _MMU_H_
#define _MMU_H_

#include "sys_regs.h"

#define PD_TABLE			0b11
#define PD_BLOCK			0b01
#define PD_ACCESS			(1 << 10)
#define BOOT_PGD_ATTR	PD_TABLE
#define BOOT_PUD_ATTR	PD_TABLE
#define BOOT_PMD_ATTR	(PD_BLOCK | PD_ACCESS)


#define PGD_SHIFT	39
#define PUD_SHIFT	30
#define PMD_SHIFT	21
#define PTE_SHIFT	12

#define PGD_BIT_SIZE	9
#define PUD_BIT_SIZE	9
#define PMD_BIT_SIZE	9
#define PTE_BIT_SIZE	12

#define PMD_BLOCK_SIZE	(1 << PMD_SHIFT)

#define BOOT_VA_START			0
#define BOOT_GPU_PERI_BASE		0x3F000000
#define BOOT_GPU_PERI_SIZE		0x1000000
#define BOOT_LOCAL_PERI_BASE	0x40000000
#define BOOT_LOCAL_PERI_SIZE	0x20000


#endif