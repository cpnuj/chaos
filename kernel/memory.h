#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "kernel/bitmap.h"
/* Use which pool */
enum pool_flags {
  PF_KERNEL = 1,
  PF_USER = 2
};

/* PDE and PTE macros */
#define PG_P_0  0
#define PG_P_1  1
#define PG_RW_R 0
#define PG_RW_W (1 << 1)
#define PG_US_S 0
#define PG_US_U (1 << 2)

struct virtual_addr {
  struct bitmap vaddr_bitmap;
  uint32_t vaddr_start;
};

extern struct pool kernel_pool, user_pool;
void* get_kernel_pages(uint32_t pg_cnt);
void mem_init(void);
#endif