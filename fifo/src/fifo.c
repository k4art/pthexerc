#include <assert.h>

#include "fifo/fifo.h"

fifo_ret_t fifo_create_for_object_size(fifo_t ** p_fifo, size_t object_size)
{
  assert(object_size > 0);
  
  return FIFO_SUCCESS;
}

fifo_ret_t fifo_destroy(fifo_t * fifo)
{
  assert(fifo != NULL);

  return FIFO_SUCCESS;
}


bool fifo_is_empty(fifo_t * fifo)
{
  assert(fifo != NULL);

  return true;
}

fifo_ret_t fifo_enqueue(fifo_t * fifo, const void * p_object)
{
  assert(fifo     != NULL);
  assert(p_object != NULL);
  
  return FIFO_SUCCESS;
}

fifo_ret_t fifo_dequeue(fifo_t * fifo, void * p_object)
{
  assert(fifo     != NULL);
  assert(p_object != NULL);

  return FIFO_SUCCESS;
}

