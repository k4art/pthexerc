#ifndef TPOOL_H
#define TPOOL_H

#include "work_queue.h"

typedef struct tpool_s tpool_t;

tpool_t * tpool_create(size_t threads_number);
void tpool_destroy(tpool_t * tpool);

void tpool_add_work(tpool_t * tpool, work_routine_t routine, void * arg);
void tpool_wait(tpool_t * tpool);

#endif

