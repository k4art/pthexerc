#ifndef PTHEXERC_FIFO
#define PTHEXERC_FIFO

#include <stdbool.h>
#include <stddef.h>

typedef struct fifo_s fifo_t;

typedef enum fifo_ret_e
{
    FIFO_SUCCESS = 0,
    FIFO_EAGAIN,
  } fifo_ret_t;


#define FIFO_CREATE_FOR(p_fifo, type) \
  fifo_create_for_object_size((p_fifo), sizeof((type)))

fifo_ret_t fifo_create_for_object_size(fifo_t ** p_fifo, size_t object_size);

fifo_ret_t fifo_destroy(fifo_t * fifo);

bool fifo_is_empty(fifo_t * fifo);

fifo_ret_t fifo_enqueue(fifo_t * fifo, const void * p_object);
fifo_ret_t fifo_dequeue(fifo_t * fifo, void * p_object);

#endif

