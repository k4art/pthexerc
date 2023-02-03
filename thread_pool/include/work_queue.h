#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <stdlib.h>
#include <stdbool.h>

#include "internals/common.h"

typedef void (* work_routine_t)(void * arg);

typedef struct work_s
{
  work_routine_t   routine;
  void           * arg;
} work_t;

typedef struct work_queue_s work_queue_t;

work_queue_t * work_queue_create(void);

void work_queue_destroy(work_queue_t * work_queue);

bool work_queue_is_empty(work_queue_t * work_queue);

err_t work_queue_add(work_queue_t * work_queue, const work_t * p_work);
err_t work_queue_remove(work_queue_t * work_queue, work_t * p_work);

void work_queue_wait_while_no_work(work_queue_t * work_queue);
void work_queue_stop_accepting(work_queue_t * work_queue);

#endif

