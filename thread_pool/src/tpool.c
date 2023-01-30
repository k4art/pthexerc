#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "errors.h"
#include "internals/malloc_c.h"

#include "work_queue.h"

#include "tpool.h"

static const size_t MAX_WORKS_WAITING = 1024;

struct tpool_s
{
  size_t         threads_number;
  work_queue_t * work_queue;

  pthread_t      threads[];
};

static void * thread_routine(void * arg)
{
  work_queue_t * work_queue = arg;

  work_t work;
  err_t  err = SUCCESS;

  while (err != ERROR_OUT_OF_SERVICE)
  {
    work_queue_wait_while_no_work(work_queue);

    while ((err = work_queue_remove(work_queue, &work)) == SUCCESS)
    {
      work.routine(work.arg);
    }
  }

  return NULL;
}

tpool_t * tpool_create(size_t threads_number)
{
  size_t size = sizeof(tpool_t) + sizeof(pthread_t) * threads_number;
  void * memory = malloc_c(size);

  tpool_t   * tpool   = memory;
  pthread_t * threads = memory + sizeof(tpool_t);

  tpool->threads_number = threads_number;

  tpool->work_queue = work_queue_create(MAX_WORKS_WAITING);

  for (size_t i = 0; i < threads_number; i++)
  {
    int ret = pthread_create(&threads[i], NULL, thread_routine, tpool->work_queue);

    CHECK_ERROR(ret, "Creating threads for tpool.");
  }

  return tpool;
}

void tpool_destroy(tpool_t * tpool)
{
  work_queue_destroy(tpool->work_queue);

  free(tpool);
}

void tpool_wait(tpool_t * tpool)
{
  work_queue_stop_accepting(tpool->work_queue);
  
  for (size_t i = 0; i < tpool->threads_number; i++)
  {
    int ret = pthread_join(tpool->threads[i], NULL);

    CHECK_ERROR(ret, "Joining the threads in a pool.");
  }
}

void tpool_add_work(tpool_t * tpool, work_routine_t routine, void * arg)
{
  work_t work =
  {
    .routine = routine,
    .arg     = arg,
  };

   work_queue_add(tpool->work_queue, &work);
}

