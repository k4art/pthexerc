#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "fifo/fifo.h"
#include "internals/malloc_c.h"

#include "errors.h"

#include "work_queue.h"

struct work_queue_s
{
  fifo_t * fifo;

  pthread_mutex_t mutex;
  pthread_cond_t  no_work_cv;

  bool stopped_accepting;
};

work_queue_t * work_queue_create()
{
  void * memory = malloc_c(sizeof(work_queue_t));

  work_queue_t * work_queue = memory;

  CHECKED(fifo_create_for_object_size(&work_queue->fifo, sizeof(work_t)));

  CHECKED(pthread_mutex_init(&work_queue->mutex, NULL));
  CHECKED(pthread_cond_init(&work_queue->no_work_cv, NULL));

  work_queue->stopped_accepting = false;

  return work_queue;
}

void work_queue_destroy(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  CHECKED(fifo_destroy(work_queue->fifo));
  
  CHECKED(pthread_mutex_destroy(&work_queue->mutex));
  CHECKED(pthread_cond_destroy(&work_queue->no_work_cv));

  free(work_queue);
}

bool work_queue_is_empty(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  bool result;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  result = fifo_is_empty(work_queue->fifo);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return result;
}

void work_queue_wait_while_no_work(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  CHECKED(pthread_mutex_lock(&work_queue->mutex));

  while (fifo_is_empty(work_queue->fifo) && !work_queue->stopped_accepting)
  {
    pthread_cond_wait(&work_queue->no_work_cv, &work_queue->mutex);
  }

  CHECKED(pthread_mutex_unlock(&work_queue->mutex));
}

err_t work_queue_add(work_queue_t * work_queue, const work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);

  err_t err = SUCCESS;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  {
    if (work_queue->stopped_accepting)
    {
      err = ERROR_OUT_OF_SERVICE;
    }
    else
    {
      fifo_ret_t ret = fifo_enqueue(work_queue->fifo, p_work);
      CHECKED(ret);
    }
  }
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return err;
}

err_t work_queue_remove(work_queue_t * work_queue, work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);

  err_t err = SUCCESS;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
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
      fifo_dequeue(work_queue->fifo, p_work);
    }
  }
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return err;
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

