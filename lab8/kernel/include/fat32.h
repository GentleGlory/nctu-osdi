#ifndef _FAT32_H
#define _FAT32_H

#include "core.h"
#include "vfs.h"

#define FAT32_ATTR_LONG_NAME	0x0F
#define FAT32_ATTR_DIRECTORY	0x10
#define FAT32_ATTR_ARCHIVE		0x20

#define FAT32_SHORT_NAME_LEN	13

#define FAT32_EOC_MIN			0x0FFFFFF8u

#pragma pack(push, 1)
struct mbr_partition_entry {
	uint8_t  status;         // 0x80 = bootable
	uint8_t  chs_first[3];
	uint8_t  type;           // FAT32: 0x0B or 0x0C
	uint8_t  chs_last[3];
	uint32_t lba_start;      // starting LBA of partition
	uint32_t num_sectors;    // partition size
};
#pragma pack(pop)


#pragma pack(push, 1)

struct fat32_bpb {
	uint8_t		jmp_boot[3];
	uint8_t		oem_name[8];
	uint16_t	bytes_per_sec;
	uint8_t		sec_per_clus;
	uint16_t	rsvd_sec_cnt;
	uint8_t		num_fats;
	uint16_t	root_ent_cnt;
	uint16_t	tot_sec_16;
	uint8_t		media;
	uint16_t	fat_sz_16;
	uint16_t	sec_per_trk;
	uint16_t	num_heads;
	uint32_t	hidd_sec;
	uint32_t	tot_sec_32;

	// FAT32 extended
	uint32_t	fat_sz_32;
	uint16_t	ext_flags;
	uint16_t	fs_ver;
	uint32_t	root_clus;
	uint16_t	fs_info;
	uint16_t	bk_boot_sec;
	uint8_t		reserved[12];
	uint8_t		drv_num;
	uint8_t		reserved1;
	uint8_t		boot_sig;
	uint32_t	vol_id;
	uint8_t		vol_lab[11];
	uint8_t		fil_sys_type[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct fat32_dirent {
	uint8_t  name[11];      // 8 + 3
	uint8_t  attr;
	uint8_t  ntres;
	uint8_t  crtTimeTenth;
	uint16_t crtTime;
	uint16_t crtDate;
	uint16_t lstAccDate;
	uint16_t fstClusHI;
	uint16_t wrtTime;
	uint16_t wrtDate;
	uint16_t fstClusLO;
	uint32_t fileSize;
};
#pragma pack(pop)

struct fat32_sb {
	uint32_t	fat_start_lba;
	uint32_t	data_start_lba;

	uint16_t	bytes_per_sec;
	uint8_t		sec_per_clus;
	uint16_t	rsvd_sec_cnt;
	uint8_t		num_fats;
	uint32_t	fat_sz;

	uint32_t	root_clus;
};


struct fat32_node {
	uint32_t	first_cluster; // 檔案或目錄的起始 cluster
	uint32_t	size;          // 檔案大小（byte）
	uint8_t		attr;          // directory / file
	char		short_name[FAT32_SHORT_NAME_LEN];// 8.3 name + '\0'
};

extern struct filesystem fat32_fs;

void fat32_init();
int fat32_setup_mount(struct filesystem* fs, struct mount* mount);

#endif
