#include "partition_manager.h"

#include "debug.h"
#include "disk.h"
#include "kernel/bitmap.h"
#include "memory.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdnull.h"
#include "super_block.h"

int32_t get_free_inode_no(struct partition_manager* pmgr);
void release_inode_no(struct partition_manager* pmgr, int32_t inode_no);
void sync_inode_no(struct partition_manager* pmgr, int32_t inode_no);
bool validate_inode_no(struct partition_manager* pmgr, uint32_t inode_no);
int32_t get_free_block_no(struct partition_manager* pmgr);
void release_block_no(struct partition_manager* pmgr, int32_t block_no);
void sync_block_btmp(struct partition_manager* pmgr);

// IMPLEMENTATION

// NOTE: fs_alloc_inode_no and fs_free_inode_no only modify inode bitmap in
// memory with no operation with disk.
int32_t get_free_inode_no(struct partition_manager* pmgr) {
  int32_t free_inode_no;
  struct bitmap* inode_btmp_ptr = &pmgr->inode_btmp;

  free_inode_no = bitmap_scan(inode_btmp_ptr, 1);
  if (free_inode_no < 0) {
    return -1;
  }

  bitmap_set(inode_btmp_ptr, free_inode_no);
  return free_inode_no;
}

void release_inode_no(struct partition_manager* pmgr, int32_t inode_no) {
  // Not allow inode_no == 0, which is the root inode no
  ASSERT(inode_no > 0);
  struct bitmap* inode_btmp_ptr = &pmgr->inode_btmp;
  bitmap_unset(inode_btmp_ptr, inode_no);
}

void sync_inode_no(struct partition_manager* pmgr, int32_t inode_no) {
  struct bitmap* inode_btmp_ptr = &pmgr->inode_btmp;
  // which block contains this inode no
  uint32_t block_off = inode_no / BLOCK_BITS;
  uint32_t btmp_lba = pmgr->sblock->inode_btmp_lba + block_off;
  // find corresponding bits block in memory
  uint32_t bytes_off = block_off * BLOCK_SIZE;
  ASSERT(bytes_off < inode_btmp_ptr->btmp_bytes_len);
  void* bits_start = inode_btmp_ptr->bits + bytes_off;
  // sync
  disk_write(pmgr->part->hd, bits_start, btmp_lba, 1);
}

bool validate_inode_no(struct partition_manager* pmgr, uint32_t inode_no) {
  struct bitmap* inode_btmp_ptr = &pmgr->inode_btmp;
  return bitmap_scan_test(inode_btmp_ptr, inode_no);
}

int32_t get_free_block_no(struct partition_manager* pmgr) {
  int32_t free_block_no;
  struct bitmap* block_btmp_ptr = &pmgr->block_btmp;

  free_block_no = bitmap_scan(block_btmp_ptr, 1);
  if (free_block_no < 0) {
    return -1;
  }

  bitmap_set(block_btmp_ptr, free_block_no);
  return free_block_no;
}

void release_block_no(struct partition_manager* pmgr, int32_t block_no) {
  struct bitmap* block_btmp_ptr = &pmgr->block_btmp;
  bitmap_unset(block_btmp_ptr, block_no);
  sync_inode_no(pmgr, block_no);
}

void sync_block_btmp(struct partition_manager* pmgr) {
  disk_write(pmgr->part->hd, pmgr->block_btmp.bits,
             pmgr->sblock->block_btmp_lba, pmgr->sblock->block_btmp_secs);
}
