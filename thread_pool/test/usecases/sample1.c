/**
 * Example: Sample1
 *
 * In this sample a bunch of works is added to the work queue just after
 * the thread pool was created. Then the main thread joins the thread pool.
 *
 * The duty to stop the thread pool (before this the main thread could not exit).
 * is given to the special thread which routine is `signal_waiter()`.
 *
 * The `signal_waiter()` will call `tpool_shutdown()` once it counters SIGINT (Ctrl+C).
 * If the thread pool finished the work given, eventually all its threads will be terminated.
 * Otherwise, the thread pool will no longer accept any other work, and once the work queue
 * become empty, all its threads will be terminated.
 *
 * After the thread pool stopped, `main()` will check all works were completed and
 * print summary.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

#include "tpool.h"

static const size_t THREADS_NUMBER    = 8;
static const size_t WORK_ITEMS_NUMBER = 1000;

static const int VALUE_INCREMENT = 1000;

static bool check_works_are_done(int * values)
{
  bool success = true;

  for (size_t i = 0; i < WORK_ITEMS_NUMBER; i++)
  {
    if (values[i] != i + VALUE_INCREMENT)
    {
      fprintf(stderr, "[FAILURE]: values[%zu] = %d\n", i, values[i]);
      success = false;
    }
  }

  return success;
}

static void work_routine(void * arg)
{
  int * value = arg;
  int   old   = *value;

  *value += VALUE_INCREMENT;

  // Not all logs might appear in stdout due to lack of synchronization
  // Absence of this log should signal lack of works in the queue or deadlock
  printf("tid=%lu, old=%d, val=%d\n", pthread_self(), old, *value);

  if (*value % 2 != 0)
  {
    usleep(100 * 1000);
  }
}

static sigset_t signal_set;

static void * signal_waiter(void * context)
{
  /* This is the routine of the thread that solely handles SIGINT */

  int sig_number = 0;
  tpool_t * tpool = (tpool_t *) context;

  while (true)
  {
    sigwait(&signal_set, &sig_number);

    if (sig_number == SIGINT)
    {
      tpool_shutdown(tpool);
      return NULL;
    }
  }
}

/**
 * Should be called before creating new threads to make them inherit sigmask.
 */
static void prepare_signal_set(void)
{
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGINT);

  pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
}

int main(void)
{
  pthread_t signal_thread;

  prepare_signal_set();

  tpool_t * tpool  = NULL;
  int     * values = NULL;
  
  tpool_create(&tpool, THREADS_NUMBER);

  values = calloc(WORK_ITEMS_NUMBER, sizeof(*values));

  /* Create thread that will shutdown thread pool on SIGINT */
  pthread_create(&signal_thread, NULL, signal_waiter, tpool);
  
  for (size_t i = 0; i < WORK_ITEMS_NUMBER; i++)
  {
    values[i] = i;
    tpool_add_work(tpool, work_routine, values + i);
  }

  tpool_join(tpool);
  pthread_join(signal_thread, NULL);
  
  /* Main thread is blocked untill all works are done */
  /* and the thread pool is shutdown. */

  bool success = check_works_are_done(values);

  if (success)
  {
    printf("\nAll works are completed!\n");
  }
  else
  {
    printf("\nFailures detected.\n");
  }

  free(values);
  tpool_destroy(tpool);

  return 0;
}

