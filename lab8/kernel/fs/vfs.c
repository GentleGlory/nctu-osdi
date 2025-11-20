#include "vfs.h"
#include "kmalloc.h"
#include "string.h"
#include "tmpfs.h"
#include "fat32.h"


static struct mount *rootfs = NULL;
static struct list_head filesystems;

static int vfs_extract_name(const char* pathname, char* name, size_t len)
{
	const char *p = pathname;
	size_t idx = 0;

	if (pathname == NULL || name == NULL || len == 0)
		return -1;

	while (*p == '/')
		p++;

	if (*p == '\0')
		return -1;

	while (*p != '\0' && *p != '/') {
		if (idx + 1 >= len)
			return -1;
		name[idx++] = *p++;
	}

	name[idx] = '\0';

	while (*p == '/')
		p++;

	if (*p != '\0')
		return -1;

	return 0;
}

void vfs_init()
{
	rootfs = kmalloc_alloc(sizeof(struct mount));
	if (rootfs == NULL) {
		printf("\rrootfs mount point alloc failed.\n");
		return;
	}

	/*
	 * Ensure the mount structure starts from a known state. Otherwise, if
	 * tmpfs_setup_mount() returns early (e.g. with the debug return at line 83
	 * in tmpfs.c enabled), the leftover garbage inside the slab allocation can
	 * leave mount->root pointing to random memory and crash userland.
	 */
	memset(rootfs, 0, sizeof(*rootfs));

	INIT_LIST_HEAD(&filesystems);

	tmpfs_init();
	fat32_init();

	//tmpfs.setup_mount(&tmpfs, rootfs);
	fat32_fs.setup_mount(&fat32_fs, rootfs);
}

int vfs_register_filesystem(struct filesystem *fs) {
	list_add_tail(&fs->list, &filesystems);
	
	return 0;
}

struct file* vfs_open(const char* pathname, int flags) {
	struct vnode *root;
	struct vnode *target = NULL;
	struct file *file = NULL;
	char name[TMPFS_MAX_NAME_LEN];
	int ret;

	if (rootfs == NULL || pathname == NULL)
		return NULL;

	root = rootfs->root;
	if (root == NULL)
		return NULL;

	ret = vfs_extract_name(pathname, name, sizeof(name));
	if (ret < 0)
		return NULL;

	if (root->v_ops == NULL)
		return NULL;

	if (root->v_ops->lookup && root->v_ops->lookup(root, &target, name) == 0) {
		/* Found existing node */
	} else if ((flags & O_CREAT) && root->v_ops->create) {
		if (root->v_ops->create(root, &target, name) != 0)
			return NULL;
	} else {
		return NULL;
	}

	if (target == NULL)
		return NULL;

	file = (struct file *)kmalloc_alloc(sizeof(struct file));
	if (file == NULL) {
		if (target != root)
			kmalloc_free(target);
		return NULL;
	}

	memset(file, 0, sizeof(struct file));
	file->vnode = target;
	file->f_pos = 0;
	file->f_ops = target->f_ops;
	file->flags = flags;

	return file;
}

int vfs_close(struct file* file) {
	if (file == NULL)
		return -1;

	if (file->vnode && file->vnode != rootfs->root)
		kmalloc_free(file->vnode);

	kmalloc_free(file);
	return 0;
}

int vfs_write(struct file* file, const void* buf, size_t len) {
	if (file == NULL || buf == NULL || file->f_ops == NULL || file->f_ops->write == NULL)
		return -1;

	return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
	if (file == NULL || buf == NULL || file->f_ops == NULL || file->f_ops->read == NULL)
		return -1;

	return file->f_ops->read(file, buf, len);
}
