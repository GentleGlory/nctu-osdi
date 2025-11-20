#include "vfs.h"
#include "tmpfs.h"
#include "kmalloc.h"
#include "string.h"

int tmpfs_write(struct file* file, const void* buf, size_t len);
int tmpfs_read(struct file* file, void* buf, size_t len);

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name);
int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name);

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount);

struct vnode_operations tmpfs_v_ops = {
	.create = tmpfs_create,
	.lookup = tmpfs_lookup,
};

struct file_operations tmpfs_f_ops = {
	.read = tmpfs_read,
	.write = tmpfs_write
};

struct filesystem tmpfs = {
	.name = "tmpfs",
	.setup_mount = tmpfs_setup_mount,
	.priv = NULL,
};

static struct vnode* tmpfs_alloc_vnode(struct vnode* ref, struct tmpfs_node* node)
{
	struct vnode* vnode = (struct vnode *)kmalloc_alloc(sizeof(struct vnode));

	if (vnode == NULL)
		return NULL;

	memset(vnode, 0, sizeof(struct vnode));
	vnode->mount = ref ? ref->mount : NULL;
	vnode->f_ops = &tmpfs_f_ops;
	vnode->v_ops = &tmpfs_v_ops;
	vnode->internal = node;

	return vnode;
}

static struct tmpfs_node* tmpfs_find_child(struct tmpfs_node* dir, const char* name)
{
	struct tmpfs_node *child;

	if (dir == NULL || name == NULL)
		return NULL;

	list_for_each_entry(child, &dir->child_list, list) {
		
		if (strcmp(child->name, name) == 0)
			return child;
	}

	return NULL;
}

void tmpfs_init()
{
	INIT_LIST_HEAD(&tmpfs.list);
}

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount)
{
	struct tmpfs_node *root_node = NULL;
	struct vnode  *root_vnode = NULL;

	root_node = (struct tmpfs_node *)kmalloc_alloc(sizeof(struct tmpfs_node));

	if (root_node == NULL)
		return -1;

	
	memset(root_node, 0, sizeof(struct tmpfs_node));
	
	INIT_LIST_HEAD(&root_node->child_list);
	INIT_LIST_HEAD(&root_node->list);

	root_node->type = TMPFS_NODE_DIR;
	root_vnode = (struct vnode *)kmalloc_alloc(sizeof(struct vnode));
	
	if (root_vnode == NULL) {
		goto failed_alloc_vnode;
	}

	memset(root_vnode, 0, sizeof(struct vnode));
	root_vnode->mount = mount;
	root_vnode->f_ops = &tmpfs_f_ops;
	root_vnode->v_ops = &tmpfs_v_ops;
	root_vnode->internal = root_node;

	mount->fs = fs;
	mount->root = root_vnode;

	return 0;

failed_alloc_vnode:
	kmalloc_free(root_node);
	return -1;
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
	struct tmpfs_node *dir = NULL;
	struct tmpfs_node *child = NULL;
	struct vnode *vnode = NULL;

	if (dir_node == NULL || target == NULL || component_name == NULL)
		return -1;

	dir = (struct tmpfs_node *)dir_node->internal;
	if (dir == NULL || dir->type != TMPFS_NODE_DIR)
		return -1;

	child = tmpfs_find_child(dir, component_name);
	if (child == NULL)
		return -1;

	vnode = tmpfs_alloc_vnode(dir_node, child);
	if (vnode == NULL)
		return -1;

	*target = vnode;
	return 0;
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
	struct tmpfs_node *dir = NULL;
	struct tmpfs_node *new_node = NULL;
	struct vnode *vnode = NULL;
	size_t name_len = 0;

	if (dir_node == NULL || target == NULL || component_name == NULL)
		return -1;

	dir = (struct tmpfs_node *)dir_node->internal;
	if (dir == NULL || dir->type != TMPFS_NODE_DIR)
		return -1;

	if (component_name[0] == '\0')
		return -1;

	while (component_name[name_len] != '\0') {
		if (name_len >= TMPFS_MAX_NAME_LEN - 1)
			return -1;
		name_len++;
	}

	if (tmpfs_find_child(dir, component_name) != NULL)
		return -1;

	new_node = (struct tmpfs_node *)kmalloc_alloc(sizeof(struct tmpfs_node));
	if (new_node == NULL)
		return -1;

	memset(new_node, 0, sizeof(struct tmpfs_node));
	INIT_LIST_HEAD(&new_node->child_list);
	INIT_LIST_HEAD(&new_node->list);
	new_node->type = TMPFS_NODE_FILE;
	memcpy(new_node->name, component_name, name_len + 1);

	list_add_tail(&new_node->list, &dir->child_list);
	dir->child_count++;

	vnode = tmpfs_alloc_vnode(dir_node, new_node);
	if (vnode == NULL) {
		list_del(&new_node->list);
		dir->child_count--;
		kmalloc_free(new_node);
		return -1;
	}

	*target = vnode;
	return 0;
}

int tmpfs_write(struct file* file, const void* buf, size_t len)
{
	struct tmpfs_node *node = NULL;
	size_t writable = 0;

	if (file == NULL || buf == NULL || file->vnode == NULL)
		return -1;

	node = (struct tmpfs_node *)file->vnode->internal;
	if (node == NULL || node->type != TMPFS_NODE_FILE)
		return -1;

	if (file->f_pos >= TMPFS_MAX_FILE_LEN)
		return 0;

	writable = TMPFS_MAX_FILE_LEN - file->f_pos;
	if (len < writable)
		writable = len;

	if (writable == 0)
		return 0;

	memcpy(node->data + file->f_pos, buf, writable);
	file->f_pos += writable;

	if (file->f_pos > node->size)
		node->size = file->f_pos;

	return writable;
}

int tmpfs_read(struct file* file, void* buf, size_t len)
{
	struct tmpfs_node *node = NULL;
	size_t readable = 0;

	if (file == NULL || buf == NULL || file->vnode == NULL)
		return -1;

	node = (struct tmpfs_node *)file->vnode->internal;
	if (node == NULL || node->type != TMPFS_NODE_FILE)
		return -1;

	if (file->f_pos >= node->size)
		return 0;

	readable = node->size - file->f_pos;
	if (len < readable)
		readable = len;

	memcpy(buf, node->data + file->f_pos, readable);
	file->f_pos += readable;

	return readable;
}


 
