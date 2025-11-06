#ifndef _MMU_H_
#define _MMU_H_

#include "sys_regs.h"

#define VA_BITS			48

#define PD_TABLE			0b11
#define PD_BLOCK			0b01
#define PD_ACCESS			(1 << 10)
#define BOOT_PGD_ATTR	PD_TABLE
#define BOOT_PUD_ATTR	PD_TABLE
#define BOOT_PMD_ATTR	(PD_BLOCK | PD_ACCESS)

#define PGD_SHIFT		39
#define PGD_SIZE		(1ULL << PGD_SHIFT)
#define PGD_MASK		(~(PGD_SIZE-1))
#define PTRS_PER_PGD	(1ULL << (VA_BITS - PGD_SHIFT))

#define PUD_SHIFT		30
#define PUD_SIZE		(1ULL << PUD_SHIFT)
#define PUD_MASK		(~(PUD_SIZE-1))
#define PTRS_PER_PUD	(1ULL << (PGD_SHIFT - PUD_SHIFT))

#define PMD_SHIFT		21
#define PMD_SIZE		(1ULL << PMD_SHIFT)
#define PMD_MASK		(~(PMD_SIZE-1))
#define PTRS_PER_PMD	(1ULL << (PUD_SHIFT - PMD_SHIFT))

#define PTE_SHIFT		12
#define PTE_SIZE		(1ULL << PTE_SHIFT)
#define PTE_MASK		(~(PTE_SIZE-1))
#define PTRS_PER_PTE	(1ULL << (PMD_SHIFT - PTE_SHIFT))


#define PGD_MAX_BIT_POS		48
#define PUD_MAX_BIT_POS		39
#define PMD_MAX_BIT_POS		29
#define PTE_MAX_BIT_POS		20

#define PGD_BIT_SIZE	9
#define PUD_BIT_SIZE	9
#define PMD_BIT_SIZE	9
#define PTE_BIT_SIZE	12

#define PAGE_SIZE		(1ULL << PTE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE-1))
#define PAGE_SHIFT		12

#define PTE_ADDR_MASK (((1ULL << (VA_BITS - PAGE_SHIFT)) - 1) << PAGE_SHIFT)

#define PAGE_ALIGN(addr) (((addr)+PAGE_SIZE-1) & PAGE_MASK)

#define BOOT_VA_START			0
#define VC_BASE_ADDRESS			0x3b400000
#define BOOT_GPU_PERI_BASE		0x3F000000
#define BOOT_GPU_PERI_SIZE		0x1000000
#define BOOT_LOCAL_PERI_BASE	0x40000000
#define BOOT_LOCAL_PERI_SIZE	0x20000

#define PAGE_TOTAL_NUM	(VC_BASE_ADDRESS / PAGE_SIZE)

#define pgd_index(addr) (((addr) >> PGD_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_offset_raw(pgd, addr) ((pgd) + pgd_index(addr))

#define pud_index(addr) ((addr) >> PUD_SHIFT & (PTRS_PER_PUD - 1))
#define pmd_index(addr) ((addr) >> PMD_SHIFT & (PTRS_PER_PMD - 1))
#define pte_index(addr) (((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

#define pgd_addr_end(addr, end)						\
({	unsigned long long __boundary = ((addr) + PGD_SIZE) & PGD_MASK;	\
	(__boundary - 1 < (end) - 1) ? __boundary : (end);		\
})

#define pud_addr_end(addr, end)						\
({	unsigned long long __boundary = ((addr) + PUD_SIZE) & PUD_MASK;	\
	(__boundary - 1 < (end) - 1) ? __boundary : (end);		\
})

#define pmd_addr_end(addr, end)						\
({	unsigned long long __boundary = ((addr) + PMD_SIZE) & PMD_MASK;	\
	(__boundary - 1 < (end) - 1) ? __boundary : (end);		\
})



#ifndef __ASM__

#include "core.h"


typedef uint64_t pgd_t;
typedef uint64_t pud_t;
typedef uint64_t pmd_t;
typedef uint64_t pte_t;

void mmu_create_pgd_mapping(pgd_t *pgdir, uint64_t phys,
		uint64_t virt, uint64_t size, uint64_t prot);


#endif

#endif