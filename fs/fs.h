#ifndef __FS_FS_H
#define __FS_FS_H

#include "stdint.h"
#include "stdbool.h"
#include "disk.h"
#include "kernel/list.h"
#include "kernel/bitmap.h"

// Layout of a disk partition:
// ----------------------------------------------------------------------------
// | bootsec | super block | block bitmap | inode bitmap | inode table | data |
// ----------------------------------------------------------------------------

#define FS_BLOCK_SECS         1 // A block consists of a sector
#define FS_BLOCK_SIZE         (FS_BLOCK_SECS * 512)
#define FS_BLOCK_BITS         (FS_BLOCK_SECS * 512 * 8)
#define FS_SUPER_BLOCK_MAGIC 0x19970322

struct super_block {
  uint32_t magic;
  uint32_t sec_cnt;
  uint32_t inode_cnt;
  uint32_t part_lba_start;

  uint32_t block_btmp_lba;
  uint32_t block_btmp_secs;

  uint32_t inode_btmp_lba;
  uint32_t inode_btmp_secs;

  uint32_t inode_table_lba;
  uint32_t inode_table_secs;

  uint32_t data_lba;
  uint32_t root_inode_no;
  uint32_t dir_entry_size;

  uint8_t pad[460];
} __attribute__ ((packed));

struct inode {
  uint32_t no; // inode no.
  uint32_t size; // dir: entry cnt, normal: file size
  uint32_t sectors[13]; // 0 - 11: data sector index, 12: extend block index
};

#define FS_INODE_BTMP_BLOCKS        1
#define FS_INODE_CNT                (FS_INODE_BTMP_BLOCKS * FS_BLOCK_BITS)
#define FS_INODE_TABLE_SIZE         (sizeof(struct inode))
#define FS_INODE_TABLES_BLOCK_CNT   (FS_BLOCK_SIZE / FS_INODE_TABLE_SIZE)

struct inode_elem {
  struct inode inode;
  struct list_elem inode_tag;
  int ref; // How many files reference this inode
};

struct fs_manager {
  struct partition* part;
  struct super_block* sblock;
  struct list inode_list; // record opened inode
  struct bitmap inode_btmp;
  struct bitmap block_btmp;
};

// default file system manager
struct fs_manager fsm_default;

#define FS_MAX_FILENAME 50
struct dir_entry {
  char filename[FS_MAX_FILENAME];
  uint32_t inode_no;
};

struct file {
  int fd_flag;
  uint32_t fd_pos;
  struct inode_elem* inode_elem;
};

void fs_init(void);

#endif
