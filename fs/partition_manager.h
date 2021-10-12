#ifndef __FS_PARTMGR_H
#define __FS_PARTMGR_H

#include "stdint.h"
#include "kernel/bitmap.h"
#include "super_block.h"

// partition_manager is responsible for managing superblock, inode bitmap and
// block bitmap for a partition
struct partition_manager {
  struct partition* part;
  struct super_block* sblock;
  struct bitmap inode_btmp;
  struct bitmap block_btmp;
};

// default partition manager
struct partition_manager cur_partition;

#define FS_INODE_NUM_SECTORS 13
#define FS_INODE_MAX_SECTORS (12 + BLOCK_SIZE / (sizeof(uint32_t)))
#define FS_INODE_EXTEND_BLOCK_INDEX 12

#define FS_INODE_BTMP_BLOCKS        1
#define FS_INODE_CNT                (FS_INODE_BTMP_BLOCKS * BLOCK_BITS)

// NOTE: 12 is the size of struct inode
#define FS_INODE_TABLE_SIZE         (12 + 4 * FS_INODE_NUM_SECTORS)
#define FS_INODE_TABLES_BLOCK_CNT   (BLOCK_SIZE / FS_INODE_TABLE_SIZE)
#define FS_EXTEND_BLOCK_CNT         (BLOCK_SIZE / (sizeof uint32))

extern int32_t get_free_inode_no(struct partition_manager* pmgr);
extern void release_inode_no(struct partition_manager* pmgr, int32_t inode_no);
extern void sync_inode_no(struct partition_manager* pmgr, int32_t inode_no);
extern int32_t get_free_block_no(struct partition_manager* pmgr);
extern void release_block_no(struct partition_manager* pmgr, int32_t block_no);
extern void sync_block_no(struct partition_manager* pmgr, int32_t block_no);

#endif