#include "fat32.h"
#include "sd.h"
#include "kmalloc.h"
#include "string.h"


int fat32_lookup(struct vnode* dir, struct vnode** target, const char* component_name);
int fat32_read(struct file* f, void* buf, size_t len);
int fat32_write(struct file* f, const void* buf, size_t len);
int fat32_create(struct vnode* dir_node, struct vnode** target, const char* component_name);

struct vnode_operations fat32_v_ops = {
	.lookup = fat32_lookup,
	.create = fat32_create,
};

struct file_operations fat32_f_ops = {
	.read = fat32_read,
	.write = fat32_write,
};

struct filesystem fat32_fs = {
	.name = "fat32",
	.setup_mount = fat32_setup_mount,
	.priv = NULL,
};

static inline struct fat32_sb *fat32_sb_from_vnode(struct vnode *vn)
{
	return (struct fat32_sb *)vn->mount->fs->priv;
}

static inline uint32_t fat32_cluster_to_lba(struct fat32_sb *sb, uint32_t clus)
{
	return sb->data_start_lba + (clus - 2) * sb->sec_per_clus;
}

static inline char fat32_toupper(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - ('a' - 'A');
	return c;
}

static int fat32_name_equal(const char *a, const char *b)
{
	while (*a != '\0' && *b != '\0') {
		if (fat32_toupper(*a) != fat32_toupper(*b))
			return 0;
		a++;
		b++;
	}

	return (*a == '\0' && *b == '\0');
}

static struct vnode *fat32_build_vnode(struct mount *mnt,
							struct fat32_node *node,
							struct vnode_operations *v_ops,
							struct file_operations *f_ops)
{
	struct vnode *vn = kmalloc_alloc(sizeof(struct vnode));
	memset(vn, 0, sizeof(*vn));
	
	vn->mount   = mnt;
	vn->internal = node;
	vn->v_ops   = v_ops;
	vn->f_ops   = f_ops;
	
	return vn;
}

static void fat32_dirname_to_str(char out[FAT32_SHORT_NAME_LEN], const uint8_t name[11])
{
	int o = 0;
	// 8 chars name
	for (int i = 0; i < 8 && name[i] != ' '; i++) {
		out[o++] = name[i];
	}
	
	// 有 extension 再加 '.'
	if (name[8] != ' ') {
		out[o++] = '.';
		for (int i = 8; i < 11 && name[i] != ' '; i++) {
			out[o++] = name[i];
		}
	}
	out[o] = '\0';
}

static uint32_t fat32_next_cluster(struct fat32_sb *sb, uint32_t clus)
{
	uint32_t fat_offset = clus * 4; // 每 entry 4 bytes
	uint32_t sec = sb->fat_start_lba + (fat_offset / sb->bytes_per_sec);
	uint32_t off = fat_offset % sb->bytes_per_sec;

	uint8_t buf[512];   // 假設 bytes_per_sec=512
	sd_readblock(sec, buf);
	
	uint32_t entry = *(uint32_t *)(buf + off);
	entry &= 0x0FFFFFFF; // 高 4 bits 保留

	if (entry >= FAT32_EOC_MIN)
		return 0;       // 0 表示沒有下一個（EOC）
		
	return entry;
}

uint32_t fat32_detect_partition()
{
	uint8_t mbr[512];
	sd_readblock(0, mbr);  // LBA 0 = MBR
	
	// partition entries start at offset 446
	struct mbr_partition_entry* p =
		(struct mbr_partition_entry*)(mbr + 446);

	for (int i = 0; i < 4; i++) {
		uint8_t type = p[i].type;

		if (type == 0x0B || type == 0x0C) {
			// FAT32 partition found
			return p[i].lba_start;
		}
	}
	
	// Not found -> error
	printf("\rNo FAT32 partition found in MBR!\n");
	return 0;
}

static uint32_t fat32_alloc_cluster(struct fat32_sb* sb)
{
	uint8_t buf[512];
	
	uint32_t fat_sectors = sb->fat_sz;
	uint32_t fat_start = sb->fat_start_lba;

	for (uint32_t s = 0; s < fat_sectors; s++) {
		sd_readblock(fat_start + s, buf);

		uint32_t* entry = (uint32_t*)buf;
		int count = sb->bytes_per_sec / 4;

		for (int i = 0; i < count; i++) {
			uint32_t cl = s * count + i;
			if (cl < 2) continue;

			if ((entry[i] & 0x0FFFFFFF) == 0) {
				// free cluster found!
				entry[i] = 0x0FFFFFFF;  // mark EOC

				sd_writeblock(fat_start + s, buf);
				return cl;
			}
		}
	}
	
	return 0; // no free cluster
}

static void fat32_set_fat_entry(struct fat32_sb* sb, uint32_t clus, uint32_t next)
{
	uint32_t fat_off = clus * 4;
	uint32_t fat_lba = sb->fat_start_lba + (fat_off / sb->bytes_per_sec);
	uint32_t fat_pos = fat_off % sb->bytes_per_sec;

	uint8_t buf[512];
	sd_readblock(fat_lba, buf);
	
	uint32_t* p = (uint32_t*)(buf + fat_pos);
	*p = next & 0x0FFFFFFF;

	sd_writeblock(fat_lba, buf);
}

static void fat32_zero_cluster(struct fat32_sb* sb, uint32_t clus, uint8_t* buf)
{
	if (!buf)
		return;

	memset(buf, 0, sb->bytes_per_sec);

	uint32_t lba_base = fat32_cluster_to_lba(sb, clus);
	for (uint8_t s = 0; s < sb->sec_per_clus; s++)
		sd_writeblock(lba_base + s, buf);
}

void fat32_update_dirent_size(struct vnode* vnode)
{
	struct fat32_node* node = vnode->internal;
	struct fat32_sb* sb = fat32_sb_from_vnode(vnode);

	uint32_t clus = sb->root_clus;

	uint8_t sec_buf[512];
	char name_buf[12];

	while (clus != 0) {
		uint32_t lba_base = fat32_cluster_to_lba(sb, clus);

		for (uint8_t s = 0; s < sb->sec_per_clus; s++) {
			sd_readblock(lba_base + s, sec_buf);

			struct fat32_dirent* ent = (struct fat32_dirent*)sec_buf;
			int n = sb->bytes_per_sec / sizeof(struct fat32_dirent);

			for (int i = 0; i < n; i++, ent++) {
				if (ent->name[0] == 0x00) return;

				if (ent->attr == FAT32_ATTR_LONG_NAME) continue;

				fat32_dirname_to_str(name_buf, ent->name);

				if (strcmp(name_buf, node->short_name) == 0) {
					ent->fileSize = node->size;
					sd_writeblock(lba_base + s, sec_buf);
					return;
				}
			}
		}
		clus = fat32_next_cluster(sb, clus);
	}
}

int fat32_setup_mount(struct filesystem* fs, struct mount* mount)
{
	uint8_t sector[512] = {0};
	struct fat32_bpb *bpb = NULL;
	struct fat32_sb *sb = NULL;
	struct fat32_node *node = NULL;
	struct vnode *root_vnode = NULL;
	uint32_t lba;

	lba = fat32_detect_partition();

	sd_readblock(lba, sector);
	bpb = (struct fat32_bpb *) sector;

	sb = (struct fat32_sb *)kmalloc_alloc(sizeof(struct fat32_sb));
	if (!sb)
		return -1;

	memset(sb, 0, sizeof(struct fat32_sb));

	sb->bytes_per_sec = bpb->bytes_per_sec;
	sb->sec_per_clus = bpb->sec_per_clus;
	sb->rsvd_sec_cnt = bpb->rsvd_sec_cnt;
	sb->num_fats = bpb->num_fats;

	sb->fat_sz = (bpb->fat_sz_16 != 0) ? bpb->fat_sz_16 : bpb->fat_sz_32;
	sb->fat_start_lba = lba + sb->rsvd_sec_cnt;
	sb->data_start_lba = sb->fat_start_lba + sb->num_fats * sb->fat_sz;

	sb->root_clus = bpb->root_clus;

	printf("\rbytes_per_sec:%u, fat lba:%u, data lba:%u\n", 
			bpb->bytes_per_sec, sb->fat_start_lba, sb->data_start_lba);

	node = (struct fat32_node *)kmalloc_alloc(sizeof(struct fat32_node));
	memset(node, 0, sizeof(struct fat32_node));
	if (!node)
		goto alloc_node_failed;

	node->first_cluster = sb->root_clus;
	node->attr = FAT32_ATTR_DIRECTORY;
	strcpy(node->short_name, "/");

	root_vnode = kmalloc_alloc(sizeof(struct vnode));
	if (!root_vnode)
		goto alloc_vnode_failed;
	memset(root_vnode, 0, sizeof(struct vnode));

	root_vnode->mount = mount;
	root_vnode->internal = node;

	root_vnode->v_ops = &fat32_v_ops;
	root_vnode->f_ops = &fat32_f_ops;

	mount->root = root_vnode;
	mount->fs = fs;

	fs->priv = sb;

	return 0;

alloc_vnode_failed:
	kmalloc_free(node);
alloc_node_failed:
	kmalloc_free(sb);

	return -1;
}

void fat32_init()
{
	INIT_LIST_HEAD(&fat32_fs.list);
}


int fat32_read(struct file* f, void* buf, size_t len)
{
	struct fat32_node *node = (struct fat32_node *)f->vnode->internal;
	struct fat32_sb *sb = fat32_sb_from_vnode(f->vnode);
	uint8_t *sec_buf = NULL;
	int ret = -1;

	if (node->attr & FAT32_ATTR_DIRECTORY)
		return -1;    // 目前不支援讀目錄

	size_t pos  = f->f_pos;
	size_t size = node->size;

	if (pos >= size)
		return 0;     // EOF

	size_t to_read = size - pos;
	if (len < to_read)
		to_read = len;
		
	size_t cluster_size = sb->bytes_per_sec * sb->sec_per_clus;
	uint32_t clus = node->first_cluster;

	size_t offset = pos;
	size_t done   = 0;
	size_t sector_size = sb->bytes_per_sec;

	sec_buf = kmalloc_alloc(sector_size);
	if (!sec_buf)
		return -1;

	// 先找到對應到 f_pos 的 cluster
	while (offset >= cluster_size) {
		clus = fat32_next_cluster(sb, clus);
		if (clus == 0) {
			ret = done;
			goto out; // 意外 EOF
		}
		offset -= cluster_size;
	}

	while (clus != 0 && done < to_read) {
		uint32_t lba_base = fat32_cluster_to_lba(sb, clus);

		for (uint8_t s = 0; s < sb->sec_per_clus && done < to_read; s++) {
			sd_readblock(lba_base + s, sec_buf);

			size_t sec_off = 0;
			size_t sec_size = sector_size;

			// 第一次可能要從 offset 中間開始
			if (offset > 0) {
				if (offset >= sec_size) {
					offset -= sec_size;
					continue;
				}
				sec_off = offset;
				sec_size -= offset;
				offset = 0;
			}

			size_t can_copy = sec_size;
			if (can_copy > (to_read - done))
				can_copy = (to_read - done);

			memcpy((uint8_t *)buf + done, sec_buf + sec_off, can_copy);
			done += can_copy;
		}

		if (done >= to_read)
			break;
		clus = fat32_next_cluster(sb, clus);
	}

	ret = done;

out:
	if (sec_buf)
		kmalloc_free(sec_buf);

	if (ret >= 0)
		f->f_pos += ret;

	return ret;
}

int fat32_write(struct file* f, const void* buf, size_t len)
{
	struct fat32_node* node = (struct fat32_node*)f->vnode->internal;
	struct fat32_sb* sb = fat32_sb_from_vnode(f->vnode);

	if (node->attr & FAT32_ATTR_DIRECTORY)
		return -1; // 無法寫入目錄

	if (len == 0)
		return 0;
	
	size_t pos = f->f_pos;
	size_t cluster_size = sb->bytes_per_sec * sb->sec_per_clus;
	size_t sector_size = sb->bytes_per_sec;

	uint32_t clus = node->first_cluster;
	size_t offset = pos;
	size_t done = 0;
	int error = 0;
	uint8_t* sec_buf = kmalloc_alloc(sector_size);

	if (!sec_buf)
		return -1;

	if (clus == 0) {
		// 檔案完全空的 → 分配一個 cluster
		clus = fat32_alloc_cluster(sb);
		if (clus == 0) {
			error = 1;
			goto out;
		}
		fat32_zero_cluster(sb, clus, sec_buf);
		node->first_cluster = clus;
	}

	while (offset >= cluster_size) {
		offset -= cluster_size;

		uint32_t next = fat32_next_cluster(sb, clus);
		if (next == 0) {
			// 需要新 cluster
			next = fat32_alloc_cluster(sb);
			if (next == 0) {
				error = 1;
				goto out;
			}
			fat32_set_fat_entry(sb, clus, next);
			fat32_zero_cluster(sb, next, sec_buf);
		}
		clus = next;
	}

	while (done < len) {
		uint32_t lba_base = fat32_cluster_to_lba(sb, clus);
		
		for (uint8_t s = 0; s < sb->sec_per_clus && done < len; s++) {
			sd_readblock(lba_base + s, sec_buf);

			size_t sec_off = 0;
			size_t sec_size = sb->bytes_per_sec;

			// 如果 offset 落在這裡
			if (offset > 0) {
				if (offset >= sec_size) {
					offset -= sec_size;
					continue;
				}
				sec_off = offset;
				sec_size -= offset;
				offset = 0;
			}

			size_t can_write = sec_size;
			if (can_write > (len - done))
				can_write = (len - done);

			memcpy(sec_buf + sec_off, (const uint8_t*)buf + done, can_write);
			sd_writeblock(lba_base + s, sec_buf);

			done += can_write;
		}

		if (done >= len)
			break;

		// 需要下一個 cluster
		uint32_t next = fat32_next_cluster(sb, clus);
		if (next == 0) {
			next = fat32_alloc_cluster(sb);
			if (next == 0) {
				error = 1;
				goto out;
			}
			fat32_set_fat_entry(sb, clus, next);
			fat32_zero_cluster(sb, next, sec_buf);
		}
		clus = next;
	}

out:
	if (done > 0) {
		f->f_pos += done;
		size_t final_size = pos + done;
		if (final_size > node->size)
			node->size = final_size;
		fat32_update_dirent_size(f->vnode);
	}

	if (sec_buf)
		kmalloc_free(sec_buf);

	if (done == 0 && error)
		return -1;

	return done;
}

int fat32_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
	struct fat32_node *dir = (struct fat32_node *) dir_node->internal;
	struct fat32_sb *sb = fat32_sb_from_vnode(dir_node);
	uint32_t clus;
	uint8_t sec_buf[512];
	char name_buf[FAT32_SHORT_NAME_LEN];

	if (!(dir->attr & FAT32_ATTR_DIRECTORY))
		return -1;

	clus = dir->first_cluster;
	
	while (clus != 0) {
		uint32_t lba_base = fat32_cluster_to_lba(sb, clus);
		
		for (uint8_t s = 0; s < sb->sec_per_clus; s++) {
			sd_readblock(lba_base + s, sec_buf);

			struct fat32_dirent *ent = (struct fat32_dirent *)sec_buf;
			int entries = sb->bytes_per_sec / sizeof(struct fat32_dirent);

			for (int i = 0; i < entries; i++, ent++) {
				// name[0] == 0x00 → 目錄結束
				if (ent->name[0] == 0x00) {
					return -1;
				}
					
					
				// 0xE5 = deleted entry
				if (ent->name[0] == 0xE5)
					continue;
				
				// 長檔名 entry
				if (ent->attr == FAT32_ATTR_LONG_NAME)
					continue;

				fat32_dirname_to_str(name_buf, ent->name);

				if (fat32_name_equal(name_buf, component_name)) {
					// 找到了
					struct fat32_node *node = kmalloc_alloc(sizeof(struct fat32_node));
					if (!node)
						return -1;
					memset(node, 0, sizeof(*node));

					uint32_t hi = ent->fstClusHI;
					uint32_t lo = ent->fstClusLO;
					node->first_cluster = (hi << 16) | lo;
					node->size = ent->fileSize;
					node->attr = ent->attr;
					strncpy(node->short_name, name_buf, sizeof(node->short_name));
					node->short_name[sizeof(node->short_name) - 1] = '\0';

					*target = fat32_build_vnode(dir_node->mount, node,
												dir_node->v_ops,
												dir_node->f_ops);
					return 0;
				}
			}
		}
		clus = fat32_next_cluster(sb, clus);
	}
	
	return -1; // not found
}

int fat32_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
	return -1;
}
