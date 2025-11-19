#include "kmalloc.h"
#include "obj_alloc.h"
#include "string.h"
#include "page.h"
#include "buddy_sys.h"

static size_t bucket_sizes[] = {
	8, 16, 32, 64, 128, 256, 512, 1024, 2048
};

#define NUM_BUCKETS (sizeof(bucket_sizes)/sizeof(bucket_sizes[0]))

static int bucket_cache_token[NUM_BUCKETS];

void kmalloc_init()
{
	for (int i = 0; i < NUM_BUCKETS; i++) {
		bucket_cache_token[i] = obj_register(bucket_sizes[i]);

		if (bucket_cache_token[i] == -1) {
			printf("\rFailed to register obj allocator:%d\n", bucket_sizes[i]);
		}
	}
}

static int kmalloc_find_bucket(size_t size) {
	for (int i = 0; i < NUM_BUCKETS; i++) {
		if (size <= bucket_sizes[i])
			return i;
	}
	return -1;
}

static int kmalloc_calc_order(size_t size)
{
	size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	int order = 0;
	while ((1ULL << order) < pages)
		order++;
	
	return order;
}

void *kmalloc_alloc(size_t size)
{
	int bucket = kmalloc_find_bucket(size);
	int pfn;
	int order;
	uint64_t phys;
	
	if (bucket >= 0) {
		int token = bucket_cache_token[bucket];
		return obj_alloc(token);
	}
	
	order = kmalloc_calc_order(size);
	pfn = buddy_sys_alloc_block(order, 0);

	if (pfn != -1) {
		phys = PAGE_FRAME_NUM_TO_PHYS(pfn);
		return PHYS_TO_KERNEL_VIRT(phys);
	}

	return NULL;
}

void kmalloc_free(void *ptr)
{
	uint64_t phy = KERNEL_VIRT_TO_PHYS((uint64_t)ptr);
	int pfn = PHYS_TO_PAGE_FRAME_NUM(phy);

	if (pfn >= PAGE_TOTAL_NUM) {
		printf("\rThe pfn should not >= PAGE_TOTAL_NUM\n");
		return;
	}

	if (pages[pfn].order == -1) {
		printf("\rThe order of the page should not be -1\n");
		return;
	}

	if (pages[pfn].used == 0) {
		printf("\rThe used property of the page should not be 0\n");
		return;
	}

	//Slab
	if (pages[pfn].type) {
		obj_free(ptr);
	} else {
		buddy_sys_free_block(pfn, pages[pfn].order);
	}
}
