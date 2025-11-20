#ifndef _TMPFS_H
#define _TMPFS_H

#include "list.h"

struct filesystem; 

extern struct filesystem tmpfs;

#define TMPFS_MAX_NAME_LEN	32
#define TMPFS_MAX_FILE_LEN	512

enum tmpfs_node_type {
	TMPFS_NODE_DIR,
	TMPFS_NODE_FILE,
};

struct tmpfs_node {
	enum tmpfs_node_type type;
	char name[TMPFS_MAX_NAME_LEN];

	int 				child_count;
	struct list_head	child_list;

	char data[TMPFS_MAX_FILE_LEN];
	size_t size;

	struct list_head	list;
};

void tmpfs_init();

#endif