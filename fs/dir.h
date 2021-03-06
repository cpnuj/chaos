#ifndef __FS_DIR_H
#define __FS_DIR_H

#include "partition_manager.h"
#include "stdint.h"
#include "super_block.h"

enum file_type {
  TYPE_DIR,
  TYPE_NORMAL,
};

#define FS_MAX_FILENAME 50
struct dir_entry {
  char filename[FS_MAX_FILENAME];
  enum file_type f_type;
  int32_t inode_no;
} __attribute__((packed));

#define DIR_ENTRY_PER_BLOCK (BLOCK_SIZE / (sizeof(struct dir_entry)))
#define DIR_MAX_ENTRY (DIR_ENTRY_PER_BLOCK * FS_INODE_TOTAL_BLOCKS)

struct dir {
  struct inode_elem* inode_elem;
  int32_t d_pos;  // iterator position
  char d_buf[BLOCK_SIZE];
};

struct dir dir_root;

extern void dir_open_root(struct partition_manager* fsm);
extern struct dir* dir_open(struct partition_manager* pmgr, int32_t inode_no);
extern int32_t dir_close(struct dir* dir);
extern struct dir_entry* dir_read(struct dir* dir);
extern bool dir_is_empty(struct dir* parent);
extern int32_t dir_create_entry(struct dir* parent, struct dir_entry* ent);
extern int32_t dir_delete_entry(struct dir* parent, int32_t inode_no);
extern int32_t dir_search(struct dir* parent, char* filename,
                          struct dir_entry* ent);

#endif
