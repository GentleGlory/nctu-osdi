#ifndef _VFS_H
#define _VFS_H

#include "core.h"
#include "list.h"

struct vnode;
struct file;
struct mount;
struct filesystem;
struct file_operations;
struct vnode_operations;

struct vnode {
	struct mount* mount;
	struct vnode_operations* v_ops;
	struct file_operations* f_ops;
	void* internal;
};

struct file {
	struct vnode* vnode;
	size_t f_pos; // The next read/write position of this file descriptor
	struct file_operations* f_ops;
	int flags;
};

struct mount {
	struct vnode* root;
	struct filesystem* fs;
};

struct filesystem {
	const char			*name;
	struct list_head	list;
	void				*priv;
	int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

#define O_CREAT		0x0001

struct file_operations {
	int (*write) (struct file* file, const void* buf, size_t len);
	int (*read) (struct file* file, void* buf, size_t len);
};

struct vnode_operations {
	int (*lookup)(struct vnode* dir_node, struct vnode** target, const char* component_name);
	int (*create)(struct vnode* dir_node, struct vnode** target, const char* component_name);
};


void vfs_init();
int vfs_register_filesystem(struct filesystem* fs);
struct file* vfs_open(const char* pathname, int flags);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);

#endif
