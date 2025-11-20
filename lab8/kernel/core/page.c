#include "page.h"
#include "string.h"
#include "lock.h"
#include "context.h"
#include "buddy_sys.h"
#include "obj_alloc.h"
#include "kmalloc.h"

struct page pages[PAGE_TOTAL_NUM];
uint64_t remain_page_num;

extern char __kernel_start[];
extern char __kernel_end[];

LIST_HEAD(free_list);

static void page_add_free_pages_to_buddy_sys(struct page ** start, struct page ** end)
{
	if ((*start) != NULL) {
		if ((*end) == NULL) {
			(*end) = (*start);
		}

		buddy_sys_add_free_pages((*start), (*end));
		(*start) = NULL;
		(*end) = NULL;
	}
}

void page_init()
{
	struct page *start = NULL, *end = NULL;
	uint64_t kernel_start_phy = KERNEL_VIRT_TO_PHYS(__kernel_start);
	uint64_t kernel_end_phy = KERNEL_VIRT_TO_PHYS(__kernel_end);
	
	kernel_start_phy &= PAGE_MASK;
	kernel_end_phy = PAGE_ALIGN(kernel_end_phy);

	remain_page_num = 0;
	buddy_sys_init();
	obj_init();
	kmalloc_init();
		
	for (int i = 0; i < PAGE_TOTAL_NUM; i++) {
		pages[i].refcount = 0;
		pages[i].used = 1;
		pages[i].reserved = 0;
		pages[i].page_num = i;
		pages[i].order = -1;
		INIT_LIST_HEAD(&pages[i].list);

		uint64_t page_phy = PAGE_FRAME_NUM_TO_PHYS(i);

		if (page_phy < kernel_start_phy || page_phy >= kernel_end_phy) {
			
			if (start == NULL) {
				start = &pages[i];
			} else {
				end = &pages[i];
			}

			remain_page_num ++;
		} else {

			page_add_free_pages_to_buddy_sys(&start, &end);
			pages[i].reserved = 1;
		}
	}

	page_add_free_pages_to_buddy_sys(&start, &end);
	printf("\rTotal page frame:%u, free page num:%llu\n", PAGE_TOTAL_NUM, remain_page_num);
}

uint64_t page_remain_page_num()
{
	return remain_page_num;
}
