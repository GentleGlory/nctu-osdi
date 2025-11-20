#ifndef _PAGE_H
#define _PAGE_H

#include "mmu.h"

#ifndef __ASM__

#include "core.h"
#include "list.h"

extern char __kernel_virtual_base[];

#define KERNEL_VIRT_TO_PHYS(addr) \
	((uint64_t)(addr) - (uint64_t)__kernel_virtual_base)

#define PHYS_TO_KERNEL_VIRT(addr) \
	((void *)((uint64_t)(addr) + (uint64_t)__kernel_virtual_base))

#define PAGE_NUM_MASK	GENMASK(PGD_MAX_BIT_POS, PTE_SHIFT)

#define PHYS_TO_PAGE_FRAME_NUM(phy)	(((phy) & PAGE_NUM_MASK) >> PTE_SHIFT)
#define PAGE_FRAME_NUM_TO_PHYS(num)	(((uint64_t)(num)) << PTE_SHIFT)

struct page {
	int					refcount;
	int					used;
	int					reserved;
	int					page_num;
	int					order;
	int					type; //slab:1 or buddy:0
	struct list_head	list;
};

extern struct page pages[PAGE_TOTAL_NUM];
extern uint64_t remain_page_num;

void page_init();
uint64_t page_remain_page_num();

#endif /* __ASM__ */

#endif
