#include "page.h"

static struct page page[PAGE_TOTAL_NUM];

extern char __kernel_start[];
extern char __kernel_end[];

LIST_HEAD(free_list);

void page_init()
{
	uint64_t kernel_start_phy = KERNEL_VIRT_TO_PHYS(__kernel_start);
	uint64_t kernel_end_phy = KERNEL_VIRT_TO_PHYS(__kernel_end);

	for (int i = 0; i < PAGE_TOTAL_NUM; i++) {
		page[i].refcount = 0;
		page[i].used = 0;
		INIT_LIST_HEAD(&page[i].list);

		uint64_t page_phy = ((uint64_t)i) << PTE_SHIFT;

		if (page_phy < kernel_start_phy || page_phy >= kernel_end_phy) {
			list_add_tail(&page[i].list, &free_list);
		} else {
			page[i].refcount = 1;
			page[i].used = 1;
		}
	}
}

struct page *page_alloc()
{
	struct page *ret = NULL;

	if (!list_empty(&free_list)) {
		ret = list_first_entry(&free_list, struct page, list);

		ret->refcount = 1;
		ret->used = 1;
	}

	return ret;
}

void page_free(struct page *page)
{
	page->refcount = 0;
	page->used = 0;
	list_add_tail(&free_list, &page->list);
}
