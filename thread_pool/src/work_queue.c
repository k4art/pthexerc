#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "fifo/fifo.h"
#include "internals/malloc_c.h"

#include "work_queue.h"

struct work_queue_s
{
  fifo_t * fifo;

  pthread_mutex_t mutex;
  pthread_cond_t  no_work_cv;

  bool stopped_accepting;
};

work_queue_t * work_queue_create(void)
{
  work_queue_t * work_queue = NULL;

  TRY_NEW(1, work_queue = malloc(sizeof(work_queue_t)));
  TRY_EOK(2, fifo_create_for_object_size(&work_queue->fifo, sizeof(work_t)));
  TRY_EOK(3, pthread_mutex_init(&work_queue->mutex, NULL));
  TRY_EOK(4, pthread_cond_init(&work_queue->no_work_cv, NULL));

  work_queue->stopped_accepting = false;

  return work_queue;

try_failure_4: pthread_mutex_destroy(&work_queue->mutex);
try_failure_3: fifo_destroy(work_queue->fifo);
try_failure_2: free(work_queue);
try_failure_1: return NULL;
}

void work_queue_destroy(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  asserting(fifo_destroy(work_queue->fifo) == FIFO_SUCCESS);
  
  asserting(pthread_mutex_destroy(&work_queue->mutex) == 0);
  asserting(pthread_cond_destroy(&work_queue->no_work_cv) == 0);

  free(work_queue);
}

bool work_queue_is_empty(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  bool result;

  asserting(pthread_mutex_lock(&work_queue->mutex) == 0);
  result = fifo_is_empty(work_queue->fifo);
  asserting(pthread_mutex_unlock(&work_queue->mutex) == 0);

  return result;
}

void work_queue_wait_while_no_work(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  asserting(pthread_mutex_lock(&work_queue->mutex) == 0);

  while (fifo_is_empty(work_queue->fifo) && !work_queue->stopped_accepting)
  {
    asserting(pthread_cond_wait(&work_queue->no_work_cv, &work_queue->mutex) == 0);
  }

  asserting(pthread_mutex_unlock(&work_queue->mutex) == 0);
}

err_t work_queue_push(work_queue_t * work_queue, const work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);

  err_t err = SUCCESS;

  asserting(pthread_mutex_lock(&work_queue->mutex) == 0);
  {
    if (work_queue->stopped_accepting)
    {
      err = ERROR_OUT_OF_SERVICE;
    }
    else
    {
      bool should_wakeup = fifo_is_empty(work_queue->fifo);
      asserting(fifo_enqueue(work_queue->fifo, p_work) == 0);

      if (should_wakeup) 
        asserting(pthread_cond_broadcast(&work_queue->no_work_cv) == 0);
    }
  }
  asserting(pthread_mutex_unlock(&work_queue->mutex) == 0);

  return err;
}

err_t work_queue_pop(work_queue_t * work_queue, work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);

  err_t err = SUCCESS;

  asserting(pthread_mutex_lock(&work_queue->mutex) == 0);
  {
    if (fifo_is_empty(work_queue->fifo) && work_queue->stopped_accepting)
    {
      err = ERROR_OUT_OF_SERVICE;
    }
    else if (fifo_is_empty(work_queue->fifo))
    {
      err = ERROR_UNDERFLOW;
    }
    else
    {
      asserting(fifo_dequeue(work_queue->fifo, p_work) == 0);
    }
  }
  asserting(pthread_mutex_unlock(&work_queue->mutex) == 0);

  return err;
}

void work_queue_stop_accepting(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  asserting(pthread_mutex_lock(&work_queue->mutex) == 0);
  {
    assert(!work_queue->stopped_accepting);

    work_queue->stopped_accepting = true;

    asserting(pthread_cond_broadcast(&work_queue->no_work_cv) == 0);
  }
  asserting(pthread_mutex_unlock(&work_queue->mutex) == 0);
}

