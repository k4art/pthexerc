#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "fifo/fifo.h"

#include "work_queue.h"

struct work_queue_s
{
  fifo_t * fifo;

  pthread_mutex_t mutex;
  pthread_cond_t  no_work_cv;

  bool stopped_accepting;
};

#define WORK_QUEUE_LOCK(queue)   MUTEX_LOCK(&queue->mutex)
#define WORK_QUEUE_UNLOCK(queue) MUTEX_UNLOCK(&queue->mutex)

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
  if (work_queue == NULL) return;

  asserting_eok(fifo_destroy(work_queue->fifo));
  
  asserting_eok(pthread_mutex_destroy(&work_queue->mutex));
  asserting_eok(pthread_cond_destroy(&work_queue->no_work_cv));

  free(work_queue);
}

err_t work_queue_wait_while_no_work(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  WORK_QUEUE_LOCK(work_queue);
  {
    while (fifo_is_empty(work_queue->fifo) && !work_queue->stopped_accepting)
    {
      asserting_eok(pthread_cond_wait(&work_queue->no_work_cv, &work_queue->mutex));
    }
  }
  WORK_QUEUE_UNLOCK(work_queue);

  return E_OK;
}

err_t work_queue_push(work_queue_t * work_queue, const work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work     != NULL);

  err_t ret = E_OK;

  WORK_QUEUE_LOCK(work_queue);

  bool should_wakeup = fifo_is_empty(work_queue->fifo);

  if (work_queue->stopped_accepting)
  {
    ret = E_BADREQ;
    goto finish;
  }

  if (fifo_enqueue(work_queue->fifo, p_work) != FIFO_SUCCESS)
  {
    ret = E_MEMALLOC;
    goto finish;
  }

  if (should_wakeup) 
  {
    if (pthread_cond_broadcast(&work_queue->no_work_cv) != 0)
    {
      ret = E_SYSFAIL;
      goto finish;
    }
  }

  assert(ret == E_OK);

finish:
  WORK_QUEUE_UNLOCK(work_queue);

  return ret;
}

err_t work_queue_pop(work_queue_t * work_queue, work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work     != NULL);

  err_t err = E_OK;

  WORK_QUEUE_LOCK(work_queue);
  {
    bool is_empty = fifo_is_empty(work_queue->fifo);
    
    if (is_empty && work_queue->stopped_accepting)
    {
      err = E_BADREQ;
    }
    else if (is_empty)
    {
      err = E_UNDERFLOW;
    }
    else
    {
      asserting_eok(fifo_dequeue(work_queue->fifo, p_work));
    }
  }
  WORK_QUEUE_UNLOCK(work_queue);

  return err;
}

err_t work_queue_stop_accepting(work_queue_t * work_queue)
{
  assert(work_queue != NULL);
  err_t err = E_OK;

  WORK_QUEUE_LOCK(work_queue);
  {
    if (!work_queue->stopped_accepting)
    {
      work_queue->stopped_accepting = true;

      if (pthread_cond_broadcast(&work_queue->no_work_cv) != 0)
      {
        err = E_SYSFAIL;
      }
    }
  }
  WORK_QUEUE_UNLOCK(work_queue);

  return err;
}

