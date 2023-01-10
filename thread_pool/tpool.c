#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "errors.h"
#include "work_queue.h"

#include "tpool.h"

struct tpool_s
{
  pthread_t    * threads;
  size_t         threads_number;
  work_queue_t   work_queue;
};

static void * thread_routine(void * arg)
{
  (void) arg;

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

  for (size_t i = 0; i < threads_number; i++)
  {
    int ret = pthread_create(&threads[i], NULL, thread_routine, NULL);

    CHECK_ERROR(ret, "Creating threads for tpool.");
  }

  return tpool;
}

void tpool_destroy(tpool_t * tpool)
{
  free(tpool);
}

