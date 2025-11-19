#ifndef _OBJ_ALLOC_H
#define _OBJ_ALLOC_H

#include "core.h"
#include "list.h"

#define MAX_OBJ_CACHES  32

struct obj_slab;
struct obj_cache;
struct obj_free_node;

struct obj_free_node {
	struct obj_free_node *next;
};

struct obj_slab {
	struct obj_cache		*cache;
	uint32_t 				free_count;
	struct list_head		cache_list;
	struct obj_free_node	*free_node_head;
};

struct obj_cache {
	size_t		obj_size;
	size_t		obj_align;
	uint32_t	objs_per_slab;
	
	struct list_head	partial;
	struct list_head	full;
};

void obj_init();
int obj_register(size_t size);
void *obj_alloc(int token);
void obj_free(void *ptr);
void obj_reclaim();


#endif