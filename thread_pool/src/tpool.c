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

static tpool_ret_t adopt_err(err_t err)
{
  switch (err)
  {
    case E_OK:        return TPOOL_SUCCESS;
    case E_SYSFAIL:   return TPOOL_ESYSFAIL;
    case E_BADREQ:    return TPOOL_EREQREJECTED;
    case E_MEMALLOC:  return TPOOL_EMEMALLOC;

    // these errors require special treatment
    case E_UNDERFLOW:
    case E_OVERFLOW:  return (assert(false), TPOOL_EREQREJECTED);
  }
}

#define ADOPT_ERR(err) adopt_err(err)

#define ARG_SHOULD_BE(expr)                    \
  do {                                         \
    bool ok = (expr);                          \
    assert(ok && "Invalid argument: " #expr);  \
    if (!(expr))                               \
      return TPOOL_EINVARG;                    \
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
  ARG_SHOULD_BE(p_tpool != NULL);
  ARG_SHOULD_BE(threads_number > 0);

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

tpool_ret_t tpool_destroy(tpool_t * tpool)
{
  ARG_SHOULD_BE(tpool != NULL);

  work_queue_destroy(tpool->work_queue);

  free(tpool);

  return TPOOL_SUCCESS;
}

tpool_ret_t tpool_add_work(tpool_t * tpool, tpool_work_routine_t routine, void * arg)
{
  ARG_SHOULD_BE(tpool   != NULL);
  ARG_SHOULD_BE(routine != NULL);

  work_t work =
  {
    .routine = routine,
    .arg     = arg,
  };

  switch (work_queue_push(tpool->work_queue, &work))
  {
    case E_OK:     return TPOOL_SUCCESS;
    case E_BADREQ: return TPOOL_EREQREJECTED;

    default: assert(!"Unexpected return code from work_queue_add()");
  }
}

tpool_ret_t tpool_shutdown(tpool_t * tpool)
{
  ARG_SHOULD_BE(tpool != NULL);

  err_t err = work_queue_stop_accepting(tpool->work_queue);

  return ADOPT_ERR(err);
}

tpool_ret_t tpool_join(tpool_t * tpool)
{
  ARG_SHOULD_BE(tpool != NULL);

  bool sysfail = false;

  for (size_t i = 0; i < tpool->threads_number; i++)
  {
    int ret = pthread_join(tpool->threads[i], NULL);

    if (ret != 0) sysfail = true;
  }

  return sysfail ? TPOOL_ESYSFAIL : TPOOL_SUCCESS;
}

tpool_ret_t tpool_join_then_destroy(tpool_t * tpool)
{
  ARG_SHOULD_BE(tpool != NULL);

  if (tpool_join(tpool) != 0)
  {
    return TPOOL_ESYSFAIL;
  }

  tpool_destroy(tpool);

  return TPOOL_SUCCESS;
}

