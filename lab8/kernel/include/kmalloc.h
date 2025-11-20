#ifndef _KMALLOC_H
#define _KMALLOC_H

#include "core.h"

void kmalloc_init();

void *kmalloc_alloc(size_t size);
void kmalloc_free(void *ptr);
#endif