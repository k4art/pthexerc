#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "work_queue.h"

#include "tpool.h"

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
  err_t  err;

  while ((err = work_queue_remove(work_queue, &work)) != ERROR_OUT_OF_SERVICE)
  {
    if (err == SUCCESS)
    {
      work.routine(work.arg);
    }
    else
    {
      work_queue_wait_while_no_work(work_queue);
    }
  }

  return NULL;
}

static size_t try_to_create_threads(size_t n, void * context, pthread_t * threads)
{
  size_t created = 0;
  int ret = 0;

  while (created < n && ret == 0)
  {
    ret = pthread_create(threads + created, NULL, thread_routine, context);

    assert(ret == 0 || ret == EAGAIN && "pthread_create() failed");

    if (ret == 0) created++;
  }

  return created;
}

tpool_ret_t tpool_create(tpool_t ** p_tpool, size_t threads_number)
{
  assert(p_tpool != NULL);

  tpool_t      * tpool = NULL;
  work_queue_t * queue = NULL;

  size_t size = sizeof(tpool_t) + sizeof(pthread_t) * threads_number;

  TRY_NEW(1, tpool = malloc(size));
  TRY_NEW(2, queue = work_queue_create());

  size_t threads_created = try_to_create_threads(threads_number, queue, tpool->threads);

  tpool->threads_number = threads_created; // NOT threads_number, see rollback
  tpool->work_queue     = queue;

  if (threads_created != threads_number) goto rollback;

  *p_tpool = tpool;

  return TPOOL_SUCCESS;

rollback:
  if (threads_created > 0)
  {
    // Here tpool is a valid thread pool,
    // but with less number of threads then requested.
    tpool_shutdown(tpool);
    tpool_join(tpool);
  }

  tpool_destroy(tpool);

  return TPOOL_ESYSFAIL;

try_failure_2: free(tpool);
try_failure_1: return TPOOL_EMEMALLOC;
}

void tpool_destroy(tpool_t * tpool)
{
  assert(tpool != NULL);

  work_queue_destroy(tpool->work_queue);

  free(tpool);
}

tpool_ret_t tpool_add_work(tpool_t * tpool, work_routine_t routine, void * arg)
{
  assert(tpool != NULL);

  work_t work =
  {
    .routine = routine,
    .arg     = arg,
  };

  switch (work_queue_add(tpool->work_queue, &work))
  {
    case SUCCESS:              return TPOOL_SUCCESS;
    case ERROR_OUT_OF_SERVICE: return TPOOL_EREQREJECTED;

    default: assert(!"Expected work_queue_add() return code");
  }
}

void tpool_shutdown(tpool_t * tpool)
{
  assert(tpool != NULL);

  work_queue_stop_accepting(tpool->work_queue);
}

void tpool_join(tpool_t * tpool)
{
  assert(tpool != NULL);

  for (size_t i = 0; i < tpool->threads_number; i++)
  {
    asserting(pthread_join(tpool->threads[i], NULL) == 0);
  }
}

void tpool_join_then_destroy(tpool_t * tpool)
{
  assert(tpool != NULL);

  tpool_join(tpool);
  tpool_destroy(tpool);
}

