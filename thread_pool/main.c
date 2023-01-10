#include <stdlib.h>
#include "tpool.h"

static const size_t THREADS_NUMBER = 4;

int main(void)
{
  tpool_t * tpool = tpool_create(THREADS_NUMBER);

  tpool_wait(tpool);
  tpool_destroy(tpool);

  return 0;
}

