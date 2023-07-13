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

#define CHECK_PARAM(expr)           \
  do {                              \
    if (!(expr))                    \
    {                               \
      fprintf(stderr, "[TPOOL_EINVARG]: invalid argument at %s(): %s\n", __FUNCTION__, #expr); \
      return TPOOL_EINVARG;         \
    }                               \
  } while (0)

static void * thread_routine(void * arg)
{
  work_queue_t * work_queue = arg;

  work_t work;
  err_t  err;

  while ((err = work_queue_pop(work_queue, &work)) != E_BADREQ)
  {
    if (err == E_OK)
    {
      work.routine(work.arg);
    }
    else
    {
      assert(err == E_UNDERFLOW);
      work_queue_wait_while_no_work(work_queue);
    }
  }

  return NULL;
}

static size_t try_to_create_threads(size_t n, void * context, pthread_t * threads)
{
  assert(n > 0);
  assert(context != NULL);
  assert(threads != NULL);
  
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
  CHECK_PARAM(p_tpool != NULL);
  CHECK_PARAM(threads_number > 0);

  tpool_t      * tpool = NULL;
  work_queue_t * queue = NULL;

  size_t size = sizeof(tpool_t) + sizeof(pthread_t) * threads_number;

  TRY_NEW(1, tpool = malloc(size));
  TRY_NEW(1, queue = work_queue_create());

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

try_failure_1:
  tpool_destroy(tpool);
  return TPOOL_EMEMALLOC;
}

tpool_ret_t tpool_destroy(tpool_t * tpool)
{
  if (tpool != NULL)
  {
    work_queue_destroy(tpool->work_queue);
    free(tpool);
  }

  return TPOOL_SUCCESS;
}

tpool_ret_t tpool_add_work(tpool_t * tpool, tpool_work_routine_t routine, void * arg)
{
  CHECK_PARAM(tpool != NULL);
  CHECK_PARAM(routine != NULL);

  work_t work =
  {
    .routine = routine,
    .arg     = arg,
  };

  switch (work_queue_push(tpool->work_queue, &work))
  {
    case E_OK:     return TPOOL_SUCCESS;
    case E_BADREQ: return TPOOL_EREQREJECTED;

    default: UNREACHABLE();
  }
}

tpool_ret_t tpool_shutdown(tpool_t * tpool)
{
  CHECK_PARAM(tpool != NULL);

  err_t err = work_queue_stop_accepting(tpool->work_queue);

  return (tpool_ret_t) err;
}

tpool_ret_t tpool_join(tpool_t * tpool)
{
  CHECK_PARAM(tpool != NULL);

  bool sysfail = false;

  for (size_t i = 0; i < tpool->threads_number; i++)
  {
    if (pthread_join(tpool->threads[i], NULL) != 0)
    {
      sysfail = true;
    }
  }

  return sysfail ? TPOOL_ESYSFAIL : TPOOL_SUCCESS;
}

tpool_ret_t tpool_join_then_destroy(tpool_t * tpool)
{
  CHECK_PARAM(tpool != NULL);

  if (tpool_join(tpool) != TPOOL_SUCCESS)
  {
    return TPOOL_ESYSFAIL;
  }

  tpool_destroy(tpool);
  return TPOOL_SUCCESS;
}

