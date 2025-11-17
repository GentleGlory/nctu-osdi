#include "obj_alloc.h"
#include "string.h"
#include "lock.h"
#include "buddy_sys.h"
#include "page.h"

struct obj_cache obj_caches[MAX_OBJ_CACHES];
uint32_t cache_cnt = 0;

static inline struct obj_slab *obj_ptr_to_slab(void *ptr)
{
	uint64_t base = (uint64_t)ptr & ~(PAGE_SIZE - 1);
	return (struct obj_slab *)base;
}

static inline void *obj_slab_obj_base(struct obj_slab *slab)
{
	return (void *)((uint64_t)slab + sizeof(struct obj_slab));
}

static struct obj_slab *obj_slab_new(struct obj_cache *cache)
{
	int pfn;
	struct obj_slab *slab;
	byte *base;
	struct obj_free_node dummy;
	struct obj_free_node *cur;
	struct obj_free_node *node;
	
	pfn = buddy_sys_alloc_block(0, 1);
	if (pfn == -1)
		return NULL;
	
	dummy.next = NULL;
	cur = &dummy;

	slab = (struct obj_slab *)PHYS_TO_KERNEL_VIRT(PAGE_FRAME_NUM_TO_PHYS(pfn));
	slab->cache = cache;
	slab->free_count = cache->objs_per_slab;
	
	base = (byte *)obj_slab_obj_base(slab);

	for (uint32_t i = 0; i < cache->objs_per_slab; i++) {
		node = (struct obj_free_node *)(base + i * cache->obj_size);
		node->next = NULL;
		cur->next = node;
		cur = cur->next;
	}
	slab->free_node_head = dummy.next;
	return slab;
}


static void *obj_slab_alloc_obj(struct obj_slab *slab)
{
	if (slab->free_count == 0)
		return NULL;
		
	struct obj_free_node *node = slab->free_node_head;
	slab->free_node_head = slab->free_node_head->next;
	slab->free_count--;
	
	return (void *)node;
}

void obj_init()
{
	for (int i = 0; i < MAX_OBJ_CACHES; i++) {
		obj_caches[i].obj_size = 0;
		obj_caches[i].obj_align = 0;
		obj_caches[i].objs_per_slab = 0;

		INIT_LIST_HEAD(&obj_caches[i].partial);
		INIT_LIST_HEAD(&obj_caches[i].full);
	}
}

int obj_register(size_t obj_size)
{
	uint64_t irq_state;
	int token = -1;
	struct obj_cache *cache = NULL;

	if (obj_size == 0) {
		printf("\robj_register, obj_size cannot be 0\n");
		return -1;
	}

	if (obj_size < sizeof(void *)) {
		obj_size = sizeof(void *);
	}

	irq_state = lock_irq_save();

	if (cache_cnt >= MAX_OBJ_CACHES) {
		printf("\robj_register: too many caches\n");
		goto failed;
	}

	cache = &obj_caches[cache_cnt];
	cache->obj_size = obj_size;
	cache->obj_align = 8;
	cache->objs_per_slab = (PAGE_SIZE - sizeof(struct obj_slab)) / obj_size;
	INIT_LIST_HEAD(& cache->partial);
	INIT_LIST_HEAD(& cache->full);

	token = cache_cnt;
	cache_cnt++;

	printf("\robj_register,token=%d, size=%u\n",
			token, obj_size);

failed:
	lock_irq_restore(irq_state);
	return token;
}

void *obj_alloc(int token)
{
	uint64_t irq_state;
	void *ret = NULL;
	struct obj_cache *cache;
	struct obj_slab *slab;

	irq_state = lock_irq_save();
	if (token < 0 || token >= cache_cnt){
		goto failed;
	}
	
	cache = &obj_caches[token];
	
	if (!list_empty(&cache->partial)) {
		slab = list_first_entry(&cache->partial, struct obj_slab, cache_list);
		ret = obj_slab_alloc_obj(slab);
		
		if (slab->free_count == 0) {
			list_del(&slab->cache_list);
			list_add(&slab->cache_list, &cache->full);
		}
		goto done;
	}
	
	slab = obj_slab_new(cache);
	if (!slab){
		goto failed;
	}

	list_add_tail(&slab->cache_list, &cache->partial);
	ret = obj_slab_alloc_obj(slab);

done:
failed:
	lock_irq_restore(irq_state);
	return ret;
}

void obj_free(void *ptr)
{
	uint64_t irq_state;
	struct obj_slab *slab;
	struct obj_cache *cache;
	struct obj_free_node *node;

	irq_state = lock_irq_save();

	slab = obj_ptr_to_slab(ptr);
	cache = slab->cache;

	node = (struct obj_free_node *)ptr;
	node->next = slab->free_node_head;
	slab->free_node_head = node;

	slab->free_count++;
	
	if (slab->free_count == 1) {
		list_del(&slab->cache_list);
		list_add(&slab->cache_list, &cache->partial);
	}

	lock_irq_restore(irq_state);
}

void obj_reclaim()
{
	uint64_t irq_state;
	struct obj_slab *slab, *tmp;
	int pfn;

	irq_state = lock_irq_save();

	for (int i = 0; i < cache_cnt; i++) {
		if (!list_empty(&obj_caches[i].partial)) {
			list_for_each_entry_safe(slab, tmp, &obj_caches[i].partial, cache_list) {
				//Empty
				if (slab->free_count == obj_caches[i].objs_per_slab) {
					list_del(&slab->cache_list);
					
					pfn = PHYS_TO_PAGE_FRAME_NUM(KERNEL_VIRT_TO_PHYS((uint64_t)slab));
					buddy_sys_free_block(pfn, 0);
				}
			}
		}
	}

	lock_irq_restore(irq_state);
}