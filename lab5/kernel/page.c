#include "page.h"
#include "string.h"
#include "lock.h"

static struct page page[PAGE_TOTAL_NUM];

extern char __kernel_start[];
extern char __kernel_end[];

LIST_HEAD(free_list);

void page_init()
{
	uint64_t kernel_start_phy = KERNEL_VIRT_TO_PHYS(__kernel_start);
	uint64_t kernel_end_phy = KERNEL_VIRT_TO_PHYS(__kernel_end);

	uint32_t cnt = 0;

	for (int i = 0; i < PAGE_TOTAL_NUM; i++) {
		page[i].refcount = 0;
		page[i].used = 0;
		page[i].page_num = i;
		INIT_LIST_HEAD(&page[i].list);

		uint64_t page_phy = PAGE_FRAME_NUM_TO_PHYS(i);

		if (page_phy < kernel_start_phy || page_phy >= kernel_end_phy) {
			list_add_tail(&page[i].list, &free_list);
			cnt ++;
		} else {
			page[i].refcount = 1;
			page[i].used = 1;
		}
	}

	printf("\rTotal page frame:%u, free page num:%u\n", PAGE_TOTAL_NUM, cnt);
}

struct page *page_alloc()
{
	struct page *ret = NULL;
	uint64_t irq_state;
	uint64_t addr;

	irq_state = lock_irq_save();

	if (!list_empty(&free_list)) {
		ret = list_first_entry(&free_list, struct page, list);
		
		list_del(&ret->list);
		lock_irq_restore(irq_state);

		ret->refcount = 1;
		ret->used = 1;

		addr = PAGE_FRAME_NUM_TO_PHYS(ret->page_num);
		addr = (uint64_t) PHYS_TO_KERNEL_VIRT(addr);

		memset((void *) addr, 0, PAGE_SIZE);
		
	} else {
		lock_irq_restore(irq_state);
	}

	return ret;
}

void page_free(struct page *page)
{
	uint64_t irq_state;
	
	irq_state = lock_irq_save();
	
	page->refcount = 0;
	page->used = 0;
	list_add_tail(&free_list, &page->list);

	lock_irq_restore(irq_state);
}

void *page_alloc_pgtable()
{
	struct page *page = NULL;
	uint64_t addr = 0;
	void *ret = NULL;

	page = page_alloc();

	if (page != NULL) {
		addr = PAGE_FRAME_NUM_TO_PHYS(page->page_num);
		ret = PHYS_TO_KERNEL_VIRT(addr);
	}

	return ret;
}