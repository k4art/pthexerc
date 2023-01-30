#include <stdlib.h>
#include <stdio.h>

#include "malloc_c.h"

void * malloc_c(size_t size)
{
  void * memory = malloc(size);

  if (memory == NULL)
  {
    fprintf(stderr, "malloc() returned NULL\n");
    fprintf(stderr, "aborting...");
    abort();
  }

  return memory;
}
