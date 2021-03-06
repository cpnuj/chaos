#ifndef __FS_INODE_H
#define __FS_INODE_H

#include "kernel/list.h"
#include "partition_manager.h"
#include "stdbool.h"
#include "stdint.h"

// NOTE: if modify fields in inode, remember to modify FS_INODE_TABLE_SIZE
struct inode {
  int32_t no;                           // inode no.
  int32_t size;                         // dir: entry cnt, normal: file size
  int32_t blocks[FS_INODE_NUM_BLOCKS];  // 0 - 11: data block index, 12: extend
                                        // block index
} __attribute__((packed));

struct inode_elem {
  struct inode inode;
  struct partition_manager* partmgr;
  struct list_elem inode_tag;
  int32_t ref;  // How many files reference this inode
};

// inode_list caches opened inodes
struct list inode_list;

extern void inode_sync(struct inode_elem* inode_elem);
extern struct inode_elem* inode_create(struct partition_manager* pmgr,
                                       uint32_t inode_no);
extern void inode_delete(struct partition_manager* pmgr, uint32_t inode_no);
extern struct inode_elem* inode_open(struct partition_manager* pmgr,
                                     uint32_t inode_no);
extern void inode_close(struct inode_elem* inode_elem);
extern int32_t inode_get_blocks(struct inode_elem* inode_elem, uint32_t nr);
extern uint32_t inode_block_used(struct inode_elem* inode_elem);
extern uint32_t inode_get_or_create_sec(struct inode_elem* inode_elem,
                                        uint32_t sec_idx);
extern uint32_t inode_idx_to_lba(struct inode_elem* inode_elem,
                                 uint32_t sec_idx);
extern int32_t inode_read(struct inode_elem* inode_elem, uint32_t sec_idx,
                          char* buf);
extern int32_t inode_write(struct inode_elem* inode_elem, uint32_t sec_idx,
                           char* buf);

#endif
