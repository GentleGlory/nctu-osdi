#include "buddy_sys.h"
#include "mmu.h"
#include "string.h"
#include "lock.h"
#include "page.h"

struct list_head free_area[MAX_ORDER + 1];

/* find buddy PFN for a block head pfn at given order */
static inline int buddy_sys_buddy_of(int pfn, int order) {
	return pfn ^ (1 << order);
}

void buddy_sys_init()
{
	for (int i = 0; i <= MAX_ORDER; i ++) {
		INIT_LIST_HEAD(&free_area[i]);
	}
}

static void buddy_sys_add_free_block(int pfn, int order)
{
	int size = 1 << order;
	if (pfn < 0 || pfn + size > PAGE_TOTAL_NUM) return;

	struct page *head = &pages[pfn];
	
	/* mark head and interior */
	head->used = 0;
	head->order = order;
	for (int i = 1; i < size; i++) {
		pages[pfn + i].used = 0;
		pages[pfn + i].order = -1; /* not head */
	}
	list_add_tail(&head->list, &free_area[order]);
}

void buddy_sys_add_free_pages(struct page *start, struct page *end)
{
	int pfn = start->page_num;
	/* treat end as inclusive so +1 makes loop exclusive upper bound */
	int end_pfn = end->page_num + 1;
	int block_size;

	while (pfn < end_pfn) {
		int placed = 0;
		for (int order = MAX_ORDER; order >= 0; order--) {
			block_size = 1 << order;
			if (pfn % block_size != 0) continue;
			if (pfn + block_size > end_pfn) continue;
			
			buddy_sys_add_free_block(pfn, order);
			pfn += block_size;
			placed = 1;
			break;
		}
		
		if (!placed) {
			/* fallback single page */
			if (!pages[pfn].reserved) {
				buddy_sys_add_free_block(pfn, 0);
				pfn++;
			}
		}
	}
}

int buddy_sys_alloc_block(int req_order, int slab)
{
	uint64_t irq_state;
	int order = 0;
	struct page *head = NULL;

	if (req_order < 0 || req_order > MAX_ORDER) return -1;
	order = req_order;
	
	irq_state = lock_irq_save();
	
	/* find first non-empty free list >= req_order */
	int found = -1;
	for (int o = order; o <= MAX_ORDER; o++) {
		if (!list_empty(&free_area[o])) {
			found = o; 
			break; 
		}
	}
	
	if (found == -1) {
		goto failed; 
	}
	
	/* pop a block from free_list[found] */
	head = list_first_entry(&free_area[found], struct page, list);
	if (!head) {
		goto failed;
	}

	list_del(&head->list);
	
	int pfn = head->page_num;
	
	/* split down to requested order */
	for (int o = found; o > req_order; o--) {
		int half = 1 << (o - 1);
		/* left = pfn, right = pfn + half */
		int right_pfn = pfn + half;
		/* mark left as temporary head for next iteration */
		pages[pfn].used = 0;
		pages[pfn].order = o - 1;
		/* mark right half as head and push to free_list[o-1] */
		buddy_sys_add_free_block(right_pfn, o - 1);
		/* note: add_free_block sets pages[right_pfn].order = o-1 and interior pages appropriately */
		/* continue with left half (pfn stays the same) */
	}
	
	/* final returned block head is pfn with order=req_order; mark as allocated (not free) */
	pages[pfn].used = 1;
	pages[pfn].order = req_order;
	pages[pfn].type = slab;
	/* mark interior pages allocated (order = -1) */
	int size = 1 << req_order;
	for (int i = 1; i < size; i++) {
		pages[pfn + i].used = 1;
		pages[pfn + i].order = -1;
		pages[pfn + i].type = slab;
	}
	remain_page_num -= size;
	printf("\rpage_alloc_block returned pfn=%d order=%d (size pages=%d)\n", pfn, req_order, size);
	lock_irq_restore(irq_state);
	return pfn;

failed:
	lock_irq_restore(irq_state);
	return -1;
}

void buddy_sys_free_block(int head_pfn, int order)
{
	int block_size;
	uint64_t irq_state;
	int cur_pfn;
	int cur_order;
	int buddy;

	if (head_pfn < 0 || head_pfn >= PAGE_TOTAL_NUM) return;
	if (order < 0 || order > MAX_ORDER) return;

	/* sanity: ensure head alignment */
	block_size = 1 << order;
	if (head_pfn % block_size != 0) {
		printf("\rbuddy_sys_free_block warning: head_pfn %d not aligned to order %d\n", head_pfn, order);
		return;
	}
	if (head_pfn + block_size > PAGE_TOTAL_NUM) {
		printf("\rbuddy_sys_free_block warning: head_pfn %d order %d exceeds limit\n", head_pfn, order);
		return;
	}
	irq_state = lock_irq_save();
	if (!pages[head_pfn].used) {
		printf("\rbuddy_sys_free_block warning: double free pfn %d order %d\n", head_pfn, order);
		lock_irq_restore(irq_state);
		return;
	}
	
	/* mark as free (tentatively) */
	pages[head_pfn].used = 0;
	pages[head_pfn].type = 0;
	pages[head_pfn].order = order;
	INIT_LIST_HEAD(&pages[head_pfn].list);
	
	for (int i = 1; i < block_size; i++) {
		pages[head_pfn + i].used = 0;
		pages[head_pfn + i].type = 0;
		pages[head_pfn + i].order = -1;
	}

	remain_page_num += block_size;

	cur_pfn = head_pfn;
	cur_order = order;
	while (cur_order < MAX_ORDER) {
		buddy = buddy_sys_buddy_of(cur_pfn, cur_order);
		if (buddy < 0 || buddy >= PAGE_TOTAL_NUM) break;
		/* buddy must be head, free, and have same order */
		if (pages[buddy].used || pages[buddy].order != cur_order) break;
		/* remove buddy from free list */
		list_del(&pages[buddy].list);
		
		/* choose new head as min(cur_pfn, buddy) */
		if (buddy < cur_pfn) cur_pfn = buddy;
		/* increase order and continue */
		cur_order++;
		/* mark merged head's metadata will be set in next loop iteration or after loop */
	}
	
	/* insert merged block into free list */
	pages[cur_pfn].used = 0;
	pages[cur_pfn].order = cur_order;
	for (int i = 1; i < (1 << cur_order); i++) {
		pages[cur_pfn + i].used = 0;
		pages[cur_pfn + i].order = -1;
	}
	list_add_tail(&pages[cur_pfn].list, &free_area[cur_order]);
	
	printf("\rbuddy_sys_free_block inserted pfn=%d order=%d\n", cur_pfn, cur_order);
	lock_irq_restore(irq_state);
}
