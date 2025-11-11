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
	int					page_num;
	struct list_head	list;
};

void page_init();
struct page *page_alloc();
void page_free(struct page *page);
void page_free_by_page_num(uint64_t page_num);

void *page_alloc_pgtable();

uint64_t page_remain_page_num();

#endif /* __ASM__ */

#endif
