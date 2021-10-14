#include "fs.h"

#include "console.h"
#include "debug.h"
#include "dir.h"
#include "disk.h"
#include "file.h"
#include "global.h"
#include "inode.h"
#include "kernel/bitmap.h"
#include "kernel/list.h"
#include "memory.h"
#include "partition_manager.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "stdnull.h"
#include "string.h"
#include "thread.h"

// Private
bool fs_load(struct partition_manager* fsm, struct partition* part);
void fs_make(struct partition_manager* fsm, struct partition* part);

static int32_t get_local_fd(void);

// Public methods

void fs_init(void);
int32_t sys_open(const char* pathname, int32_t flags);
int32_t sys_write(int32_t fd, const void* buf, int32_t size);
int32_t sys_read(int32_t fd, void* buf, int32_t size);

// Implementation

bool fs_load(struct partition_manager* fsm, struct partition* part) {
  ASSERT(part != NULL)
  fsm->part = part;

  struct super_block* sblock;
  sblock = (struct super_block*)sys_malloc(sizeof(struct super_block));

  // read super block
  disk_read(part->hd, sblock, part->lba_start + 1, 1);

  if (sblock->magic != SUPER_BLOCK_MAGIC) {
    sys_free(sblock);
    return false;
  }

  fsm->sblock = sblock;

  // Load inode bitmap
  struct bitmap* inode_btmp_ptr = &fsm->inode_btmp;
  inode_btmp_ptr->btmp_bytes_len = sblock->inode_btmp_secs * 512;
  inode_btmp_ptr->bits = (uint8_t*)sys_malloc(inode_btmp_ptr->btmp_bytes_len);
  disk_read(part->hd, inode_btmp_ptr->bits, sblock->inode_btmp_lba,
            sblock->inode_btmp_secs);

  // Load block bitmap
  struct bitmap* block_btmp_ptr = &fsm->block_btmp;
  block_btmp_ptr->btmp_bytes_len = sblock->block_btmp_secs * 512;
  block_btmp_ptr->bits = (uint8_t*)sys_malloc(block_btmp_ptr->btmp_bytes_len);
  disk_read(part->hd, block_btmp_ptr->bits, sblock->block_btmp_lba,
            sblock->block_btmp_secs);

  return true;
}

// FIXME: check whether malloc in fs_make use kmalloc
void fs_make(struct partition_manager* fsm, struct partition* part) {
  ASSERT(part != NULL)
  fsm->part = part;

  // Init super block
  struct super_block* sblock;
  // FIXME: Currently we use fs_make before we get into user mode, it is better
  // to use a new malloc call for kernel mode
  sblock = (struct super_block*)sys_malloc(sizeof(struct super_block));

  sblock->magic = SUPER_BLOCK_MAGIC;
  sblock->sec_cnt = part->sec_cnt;
  sblock->inode_cnt = FS_INODE_CNT;
  sblock->part_lba_start = part->lba_start;

  sblock->block_btmp_lba = part->lba_start + 1 + BLOCK_SECS;
  sblock->block_btmp_secs =
      DIV_ROUND_UP(part->sec_cnt / BLOCK_SECS, BLOCK_BITS) * BLOCK_SECS;

  sblock->inode_btmp_lba = sblock->block_btmp_lba + sblock->block_btmp_secs;
  sblock->inode_btmp_secs = DIV_ROUND_UP(FS_INODE_CNT, BLOCK_BITS) * BLOCK_SECS;

  sblock->inode_table_lba = sblock->inode_btmp_lba + sblock->inode_btmp_secs;
  sblock->inode_table_secs =
      DIV_ROUND_UP(FS_INODE_CNT, FS_INODE_TABLES_BLOCK_CNT) * BLOCK_SECS;

  sblock->data_lba = sblock->inode_table_lba + sblock->inode_table_secs;
  sblock->root_inode_no = 0;
  // TODO: init dir_entry_size

  // flush super block
  disk_write(part->hd, sblock, part->lba_start + 1, 1);

  fsm->sblock = sblock;

  // Init inode bitmap
  struct bitmap* inode_btmp_ptr = &fsm->inode_btmp;
  inode_btmp_ptr->btmp_bytes_len = sblock->inode_btmp_secs * 512;
  inode_btmp_ptr->bits = (uint8_t*)sys_malloc(inode_btmp_ptr->btmp_bytes_len);
  bitmap_init(inode_btmp_ptr);

  // set root inode no
  bitmap_set(inode_btmp_ptr, sblock->root_inode_no);

  // flush all inode bitmap
  disk_write(part->hd, inode_btmp_ptr->bits, sblock->inode_btmp_lba,
             sblock->inode_btmp_secs);

  // Init block bitmap
  struct bitmap* block_btmp_ptr = &fsm->block_btmp;
  block_btmp_ptr->btmp_bytes_len = sblock->block_btmp_secs * 512;
  block_btmp_ptr->bits = (uint8_t*)sys_malloc(block_btmp_ptr->btmp_bytes_len);
  bitmap_init(block_btmp_ptr);

  // Set used block
  int32_t i, used_block;
  used_block = sblock->data_lba - sblock->part_lba_start;
  for (i = 0; i < used_block; i++) {
    bitmap_set(block_btmp_ptr, i);
  }

  // Our block bitmap size may be larger than actual block count,
  // so we need to set the tail non-exist block index to be used.
  int32_t block_btmp_size = block_btmp_ptr->btmp_bytes_len * 8;
  for (i = sblock->sec_cnt; i < block_btmp_size; i++) {
    bitmap_set(block_btmp_ptr, i);
  }

  // flush all block bitmap
  disk_write(part->hd, block_btmp_ptr->bits, sblock->block_btmp_lba,
             sblock->block_btmp_secs);

  // init root dir
  struct inode_elem root_inode_elem;
  root_inode_elem.partmgr = fsm;
  struct inode* root_inode = &root_inode_elem.inode;
  root_inode->no = sblock->root_inode_no;
  root_inode->size = 0;

  for (i = 0; i < FS_INODE_NUM_BLOCKS; i++) {
    root_inode->blocks[i] = 0;
  }

  inode_sync(&root_inode_elem);

  return;
}

void fs_init(void) {
  printf("fs_init start\n");
  ASSERT(!list_empty(&disk_partitions));
  struct list_elem* part_tag = list_top(&disk_partitions);
  struct partition* part;
  part = elem2entry(struct partition, tag, part_tag);

  // Init opened inode list
  list_init(&inode_list);

  if (!fs_load(&cur_partition, part)) {
    printf("  make default file system\n");
    fs_make(&cur_partition, part);
  }

  dir_open_root(&cur_partition);
  file_table_init();

  printf("  mount %s as default file system\n", cur_partition.part->name);
  printf("fs_init done\n");
}

static int32_t get_local_fd(void) {
  struct task_struct* cur = running_thread();

  int32_t i;
  for (i = 3; i < MAX_PROC_OPEN_FD; i++) {
    if (cur->fd_table[i] == -1) {
      return i;
    }
  }

  return -1;
}

int32_t sys_open(const char* pathname, int32_t flags) {
  int32_t fd = get_local_fd();
  if (fd < 0) {
    printf("use all fd\n");
    return -1;
  }

  char path[FS_MAX_FILENAME];
  strcpy(path, pathname + 1);

  struct task_struct* cur = running_thread();
  struct dir* cur_dir = &dir_root;
  struct dir_entry de;

  char* delim = strrchr(path, '/');
  // in current path
  if (delim == NULL) {
    // found
    if (dir_search(cur_dir, path, &de) != -1) {
      int32_t global_fd = file_open(de.inode_no, flags);
      if (global_fd < 0) {
        return -1;
      }
      // install fd
      cur->fd_table[fd] = global_fd;
      return fd;
    }

    // not found
    if (flags & O_CREATE) {
      int32_t global_fd = file_create(cur_dir, path);
      if (global_fd < 0) {
        return -1;
      }
      // install fd
      cur->fd_table[fd] = global_fd;
      return fd;
    } else {
      return -1;
    }
  }

  // search for last dir
  struct dir last_dir;
  *delim = '\0';
  if (dir_search(cur_dir, path, &de) < 0) {
    printf("cannot create %s: no such file or directory\n");
    return -1;
  }

  last_dir.inode_elem = inode_open(cur_dir->inode_elem->partmgr, de.inode_no);
  if (last_dir.inode_elem == NULL) {
    return -1;
  }

  if (dir_search(&last_dir, delim + 1, &de) != -1) {
    int32_t global_fd = file_open(de.inode_no, flags);
    if (global_fd < 0) {
      return -1;
    }
    cur->fd_table[fd] = global_fd;
    return fd;
  }

  if (flags & O_CREATE) {
    int32_t global_fd = file_create(cur_dir, path);
    if (global_fd < 0) {
      return -1;
    }
    cur->fd_table[fd] = global_fd;
    return fd;
  }

  return -1;
}

int32_t sys_write(int32_t fd, const void* buf, int32_t size) {
  struct task_struct* cur = running_thread();
  int32_t global_fd = cur->fd_table[fd];
  if (global_fd < 1) {
    printf("invalid fd\n");
    return 0;
  }

  // stdout and stderr
  if (global_fd == 1 || global_fd == 2) {
    console_put_str((char*)buf);
    return strlen((char*)buf);
  }

  return file_write(global_fd, buf, size);
}

int32_t sys_read(int32_t fd, void* buf, int32_t size) {
  struct task_struct* cur = running_thread();
  int32_t global_fd = cur->fd_table[fd];

  // stdout and stderr
  if (global_fd == 1 || global_fd == 2) {
    printf("invalid file for read\n");
    return 0;
  }

  return file_read(global_fd, buf, size);
}
