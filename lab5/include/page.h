#ifndef _PAGE_H
#define _PAGE_H

#define PAGE_TOTAL_NUM	1024

#ifndef __ASM__

#include "core.h"
#include "mmu.h"
#include "list.h"

extern char __kernel_virtual_base[];

#define KERNEL_VIRT_TO_PHYS(addr) \
	((uint64_t)(addr) - (uint64_t)__kernel_virtual_base)

#define PHYS_TO_KERNEL_VIRT(addr) \
	((void *)((uint64_t)(addr) + (uint64_t)__kernel_virtual_base))

#define PAGE_SIZE		(1 << PTE_SHIFT)
#define PAGE_NUM_MASK	GENMASK(PGD_MAX_BIT_POS, PTE_SHIFT)

#define PAGE_FRAME_NUM(phy)	(((phy) & PAGE_NUM_MASK) >> PTE_SHIFT)

struct page {
	int					refcount;
	int					used;
	struct list_head	list;
};

void page_init();
struct page *page_alloc();
void page_free(struct page *page);

#endif /* __ASM__ */

#endif
