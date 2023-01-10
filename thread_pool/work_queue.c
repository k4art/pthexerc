#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "work_queue.h"

struct work_queue_s
{
  work_t * circular_buffer;
  size_t   capacity;
  size_t   head;
  size_t   tail;
};

static size_t inc_mod(size_t n, size_t mod)
{
  return (n + 1) % mod;
}

static bool work_queue_is_empty(work_queue_t * work_queue)
{
  return work_queue->head == work_queue->capacity;
}

work_queue_t * work_queue_create(size_t capacity)
{
  size_t   size   = sizeof(work_queue_t) + sizeof(work_t) * capacity;
  void   * memory = malloc(size);

  work_queue_t * work_queue = memory;

  work_queue->circular_buffer = memory + sizeof(work_queue_t);
  work_queue->capacity = capacity;

  work_queue->head = capacity; /* empty condition*/
  work_queue->tail = 0;

  return work_queue;
}

void work_queue_destroy(work_queue_t * work_queue)
{
  free(work_queue);
}

bool work_queue_is_full(const work_queue_t * work_queue)
{
  return work_queue->tail == work_queue->head;
}

void work_queue_add(work_queue_t * work_queue, const work_t * p_work)
{
  assert(!work_queue_is_full(work_queue));

  size_t idx = work_queue->tail;

  memcpy(&work_queue->circular_buffer[idx], p_work, sizeof(work_t));

  work_queue->tail = inc_mod(idx, work_queue->capacity);

  if (work_queue_is_empty(work_queue))
  {
    work_queue->head = idx;
  }
}

