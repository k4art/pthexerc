#include <stdlib.h>
#include <stdalign.h>
#include <assert.h>
#include <string.h>

#include "fifo/fifo.h"

typedef struct fifo_node_s fifo_node_t;

struct fifo_node_s
{
  fifo_node_t * next;

  /* object of unknown size */
};

struct fifo_s
{
  fifo_node_t * head;
  fifo_node_t * tail;

  size_t object_size;
};

#define MALLOC_OR_RETURN_EAGAIN(p_memory, size) \
  do { if ((p_memory = malloc(size)) == NULL) return FIFO_EAGAIN; } while(0)

static void * fifo_node_object_begin(fifo_node_t * fifo_node)
{
  size_t align = alignof(max_align_t);
  size_t size  = sizeof(fifo_node_t);

  size_t possible_padding = align - (size % align);

  size_t begin = size + (possible_padding % align);

  return (void *) fifo_node + begin;
}

fifo_ret_t fifo_create_for_object_size(fifo_t ** p_fifo, size_t object_size)
{
  assert(object_size > 0 && "zero size is not supported");

  fifo_t * fifo = NULL;

  MALLOC_OR_RETURN_EAGAIN(fifo, sizeof(fifo_t));

  fifo->head = NULL;
  fifo->tail = NULL;

  fifo->object_size = object_size;

  assert(fifo_is_empty(fifo));

  *p_fifo = fifo;
  
  return FIFO_SUCCESS;
}

fifo_ret_t fifo_destroy(fifo_t * fifo)
{
  assert(fifo != NULL);

  fifo_node_t * node = NULL;
  fifo_node_t * next = fifo->head;

  while (next != NULL)
  {
    node = next;
    next = node->next;

    free(node);
  }

  free(fifo);

  return FIFO_SUCCESS;
}


bool fifo_is_empty(fifo_t * fifo)
{
  assert(fifo != NULL);

  // (head == NULL) if and only if (tail == NULL)
  assert((fifo->head == NULL) == (fifo->tail == NULL));

  return fifo->head == NULL;
}

fifo_ret_t fifo_enqueue(fifo_t * fifo, const void * p_object)
{
  assert(fifo     != NULL);
  assert(p_object != NULL);

  fifo_node_t * node = NULL;

  MALLOC_OR_RETURN_EAGAIN(node, fifo->object_size + sizeof(union { fifo_node_t a; max_align_t b; }));

  node->next = NULL;

  memcpy(fifo_node_object_begin(node), p_object, fifo->object_size);

  if (fifo_is_empty(fifo))
  {
    fifo->head = node;
    fifo->tail = node;
  }
  else
  {
    fifo->tail->next = node;
    fifo->tail       = node;
  }
  
  return FIFO_SUCCESS;
}

fifo_ret_t fifo_dequeue(fifo_t * fifo, void * p_object)
{
  assert(fifo     != NULL);
  assert(p_object != NULL);

  assert(!fifo_is_empty(fifo) && "fifo should be checked manualy if it is empty");

  fifo_node_t * first_out = fifo->head;

  memcpy(p_object, fifo_node_object_begin(first_out), fifo->object_size);

  if (fifo->head == fifo->tail) // if it is the last element
  {
    fifo->head = NULL;
    fifo->tail = NULL;

    assert(fifo_is_empty(fifo));
  }
  else
  {
    fifo->head = fifo->head->next;
  }

  free(first_out);

  return FIFO_SUCCESS;
}

