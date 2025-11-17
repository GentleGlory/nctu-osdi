#ifndef _BUDDY_SYS_H
#define _BUDDY_SYS_H

#include "core.h"
#include "list.h"

struct page;

#define MAX_ORDER	10

void buddy_sys_init();
void buddy_sys_add_free_pages(struct page *start, struct page *end);
int buddy_sys_alloc_block(int req_order, int slab);
void buddy_sys_free_block(int head_pfn, int order);

#endif