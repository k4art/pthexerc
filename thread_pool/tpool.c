#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "errors.h"
#include "work_queue.h"

#include "tpool.h"

static const size_t MAX_WORKS_WAITING = 128;

struct tpool_s
{
  pthread_t    * threads;
  size_t         threads_number;
  work_queue_t * work_queue;
};

static void * thread_routine(void * arg)
{
  work_queue_t * work_queue = arg;

  // wait enqueueing works
  sleep(1);

  work_t work;
  while (!work_queue_is_empty(work_queue))
  {
    work_queue_remove(work_queue, &work);

    work.routine(work.arg);
  }

  return NULL;
}

tpool_t * tpool_create(size_t threads_number)
{
  size_t size = sizeof(tpool_t) + sizeof(pthread_t) * threads_number;
  void * memory = malloc(size);

  tpool_t   * tpool   = memory;
  pthread_t * threads = memory + sizeof(tpool_t);

  tpool->threads        = threads;
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

