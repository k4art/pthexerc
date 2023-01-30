#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "tpool.h"

static const size_t THREADS_NUMBER    = 8;
static const size_t WORK_ITEMS_NUMBER = 1000;

static const int VALUE_INCREMENT = 1000;

static void work_routine(void * arg)
{
  int * value = arg;
  int   old   = *value;

  *value += VALUE_INCREMENT;

#if defined(LOG_WORKS)
  // Not all logs might appear in stdout due to lack of synchronization
  printf("tid=%lu, old=%d, val=%d\n", pthread_self(), old, *value);
#endif

  if (*value % 2 != 0)
  {
    usleep(100 * 1000);
  }
}

int main(void)
{
  tpool_t * tpool  = tpool_create(THREADS_NUMBER);
  int     * values = calloc(WORK_ITEMS_NUMBER, sizeof(*values));

  for (size_t i = 0; i < WORK_ITEMS_NUMBER; i++)
  {
    values[i] = i;
    tpool_add_work(tpool, work_routine, values + i);
  }

  tpool_shutdown(tpool);
  tpool_join(tpool);

  bool failed = false;
  for (size_t i = 0; i < WORK_ITEMS_NUMBER; i++)
  {
    if (values[i] != i + VALUE_INCREMENT)
    {
      fprintf(stderr, "[FAILURE]: values[%zu] = %d", i, values[i]);
      failed = true;
    }
  }

  if (failed)
  {
    printf("\nFailures detected.\n");
  }
  else
  {
    printf("\nAll works are completed!\n");
  }

  free(values);
  tpool_destroy(tpool);

  return 0;
}

