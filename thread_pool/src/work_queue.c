#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "errors.h"
#include "internals/malloc_c.h"

#include "work_queue.h"

struct work_queue_s
{
  size_t  capacity;
  size_t  head;
  size_t  tail;

  pthread_mutex_t mutex;
  pthread_cond_t  no_work_cv;

  bool stopped_accepting;

  work_t circular_buffer[];
};

static size_t inc_mod(size_t n, size_t mod)
{
  assert(mod != 0);

  return (n + 1) % mod;
}

static bool is_empty(work_queue_t * work_queue)
{
  /* Head is the index of the first, unless queue is empty. */
  /* In which case, head is set to the capacity value. */
  return work_queue->head == work_queue->capacity;
}

static bool is_full(work_queue_t * work_queue)
{
  return work_queue->tail == work_queue->head;
}

static err_t add_to_queue(work_queue_t * work_queue, const work_t * p_work)
{
  if (work_queue->stopped_accepting)
  {
    return ERROR_OUT_OF_SERVICE;
  }

  if (is_full(work_queue))
  {
    return ERROR_OVERFLOW;
  }

  size_t idx = work_queue->tail;

  memcpy(&work_queue->circular_buffer[idx], p_work, sizeof(*p_work));

  if (is_empty(work_queue)) /* adding first element */
  {
    work_queue->head = idx;
    pthread_cond_broadcast(&work_queue->no_work_cv);
  }

  work_queue->tail = inc_mod(idx, work_queue->capacity);
  
  return SUCCESS;
}

static err_t remove_from_queue(work_queue_t * work_queue, work_t * p_work)
{
  if (is_empty(work_queue))
  {
    if (work_queue->stopped_accepting)
    {
      return ERROR_OUT_OF_SERVICE;
    }
    else
    {
      return ERROR_UNDERFLOW;
    }
  }

  size_t idx = work_queue->head;
  
  memcpy(p_work, &work_queue->circular_buffer[idx], sizeof(*p_work));

  work_queue->head = inc_mod(idx, work_queue->capacity);

  if (is_full(work_queue)) /* underflow */
  {
    work_queue->head = work_queue->capacity;

    assert(is_empty(work_queue));
  }

  return SUCCESS;
}

work_queue_t * work_queue_create(size_t capacity)
{
  assert(capacity > 0);
  
  size_t   size   = sizeof(work_queue_t) + sizeof(work_t) * capacity;
  void   * memory = malloc_c(size);

  work_queue_t * work_queue = memory;

  work_queue->capacity = capacity;

  work_queue->head = capacity; /* empty condition */
  work_queue->tail = 0;

  CHECKED(pthread_mutex_init(&work_queue->mutex, NULL));
  CHECKED(pthread_cond_init(&work_queue->no_work_cv, NULL));

  work_queue->stopped_accepting = false;

  assert(is_empty(work_queue));
  
  return work_queue;
}

void work_queue_destroy(work_queue_t * work_queue)
{
  assert(work_queue != NULL);
  
  CHECKED(pthread_mutex_destroy(&work_queue->mutex));
  CHECKED(pthread_cond_destroy(&work_queue->no_work_cv));

  free(work_queue);
}

bool work_queue_is_empty(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  bool result;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  result = is_empty(work_queue);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return result;
}

bool work_queue_is_full(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  bool result;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  result = is_full(work_queue);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return result;
}

void work_queue_wait_while_no_work(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  CHECKED(pthread_mutex_lock(&work_queue->mutex));

  while (is_empty(work_queue) || !work_queue->stopped_accepting)
  {
    pthread_cond_wait(&work_queue->no_work_cv, &work_queue->mutex);
  }

  CHECKED(pthread_mutex_unlock(&work_queue->mutex));
}

err_t work_queue_add(work_queue_t * work_queue, const work_t * p_work)
{
  assert(work_queue != NULL);

  err_t ret_err;
  
  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  ret_err = add_to_queue(work_queue, p_work);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return ret_err;
}

err_t work_queue_remove(work_queue_t * work_queue, work_t * p_work)
{
  assert(work_queue != NULL);

  err_t ret_err;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  ret_err = remove_from_queue(work_queue, p_work);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return ret_err;
}

void work_queue_stop_accepting(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  CHECKED(pthread_mutex_lock(&work_queue->mutex));

  assert(!work_queue->stopped_accepting);

  work_queue->stopped_accepting = true;

  pthread_cond_broadcast(&work_queue->no_work_cv);

  CHECKED(pthread_mutex_unlock(&work_queue->mutex));
}

