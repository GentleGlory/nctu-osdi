#ifndef _IMAGE_LOADER_H
#define _IMAGE_LOADER_H

#include "core.h"

#define IMAGE_LOADER_DEFAULT_ADDRESS	0x80000
#define DEFAULT_STACK_SIZE				131072 //128KB
#define R_AARCH64_RELATIVE				1027

struct _rel_info
{
	uint64_t	offset;
	uint64_t	info;
	int64_t		addend;
};


void load_image();

#endif