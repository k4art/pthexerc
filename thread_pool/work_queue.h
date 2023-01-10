#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

typedef void (* work_routine_t)(void * arg);

typedef struct work_s
{
  work_routine_t   routine;
  void           * arg;
} work_t;

typedef struct work_queue_s
{
  work_t * circular_buffer;
  size_t   front;
  size_t   back;
} work_queue_t;

#endif

