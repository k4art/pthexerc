#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <stdlib.h>
#include <stdbool.h>

typedef void (* work_routine_t)(void * arg);

typedef struct work_s
{
  work_routine_t   routine;
  void           * arg;
} work_t;

typedef struct work_queue_s work_queue_t;

work_queue_t * work_queue_create(size_t capacity);

void work_queue_destroy(work_queue_t * work_queue);

bool work_queue_is_full(work_queue_t * work_queue);
bool work_queue_is_empty(work_queue_t * work_queue);

void work_queue_add(work_queue_t * work_queue, const work_t * p_work);
void work_queue_remove(work_queue_t * work_queue, work_t * p_work);

#endif

