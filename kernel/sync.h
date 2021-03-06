#ifndef __KERNEL_SYNC_H
#define __KERNEL_SYNC_H

#include "kernel/list.h"
#include "stdint.h"
#include "thread.h"

typedef struct {
  uint8_t value;
  struct list waiters;
} sem_t;

void sem_init(sem_t* sem, uint8_t value);
void sem_wait(sem_t* sem);
void sem_post(sem_t* sem);

typedef struct {
  struct task_struct* holder; /* lock holder*/
  sem_t sem;
  uint32_t holder_repeat_nr;
} lock_t;

void lock_init(lock_t* lock);
void lock_acquire(lock_t* lock);
void lock_release(lock_t* lock);

typedef struct {
  lock_t* lock;
  struct list waitq;
} cond_t;

void cond_init(cond_t* cond, lock_t* lock);
void cond_wait(cond_t* cond);
void cond_signal(cond_t* cond);

#endif
