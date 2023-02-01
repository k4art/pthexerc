#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "errors.h"
#include "internals/malloc_c.h"

#include "work_queue.h"

typedef struct queue_node_s queue_node_t;

struct queue_node_s
{
  queue_node_t * next;
  work_t         work;
};

struct work_queue_s
{
  queue_node_t * head;
  queue_node_t * tail;

  pthread_mutex_t mutex;
  pthread_cond_t  no_work_cv;

  bool stopped_accepting;
};

static bool is_empty(work_queue_t * work_queue)
{
  assert(work_queue != NULL);
  
  bool head_is_null = work_queue->head == NULL;

  /* (head == NULL) if and only if (tail == NULL) */
  assert(head_is_null == (work_queue->tail == NULL));

  return head_is_null;
}

static err_t add_to_queue(work_queue_t * work_queue, const work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);

  if (work_queue->stopped_accepting)
  {
    return ERROR_OUT_OF_SERVICE;
  }

  queue_node_t * node = malloc_c(sizeof(queue_node_t));

  node->next = NULL;
  node->work = *p_work;

  if (is_empty(work_queue)) /* adding first element */
  {
    work_queue->head = work_queue->tail = node;

    pthread_cond_broadcast(&work_queue->no_work_cv);
  }
  else
  {
    work_queue->tail->next = node;
    work_queue->tail       = node;
  }
  
  return SUCCESS;
}

static err_t remove_from_queue(work_queue_t * work_queue, work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);
  
  if (is_empty(work_queue))
  {
    if (work_queue->stopped_accepting)
    {
      return ERROR_OUT_OF_SERVICE;
    }
    else
    {
      return ERROR_UNDERFLOW;
    }
  }

  queue_node_t * front = work_queue->head;

  *p_work = front->work;

  if (work_queue->head == work_queue->tail) /* removed the last element */
  {
    work_queue->head = work_queue->tail = NULL;
  }
  else
  {
    work_queue->head = front->next;
  }

  free(front);

  return SUCCESS;
}

work_queue_t * work_queue_create()
{
  void * memory = malloc_c(sizeof(work_queue_t));

  work_queue_t * work_queue = memory;

  work_queue->head = NULL;
  work_queue->tail = NULL;

  CHECKED(pthread_mutex_init(&work_queue->mutex, NULL));
  CHECKED(pthread_cond_init(&work_queue->no_work_cv, NULL));

  work_queue->stopped_accepting = false;

  assert(is_empty(work_queue));
  
  return work_queue;
}

void work_queue_destroy(work_queue_t * work_queue)
{
  assert(work_queue != NULL);
  
  CHECKED(pthread_mutex_destroy(&work_queue->mutex));
  CHECKED(pthread_cond_destroy(&work_queue->no_work_cv));

  free(work_queue);
}

bool work_queue_is_empty(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  bool result;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  result = is_empty(work_queue);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return result;
}

void work_queue_wait_while_no_work(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  CHECKED(pthread_mutex_lock(&work_queue->mutex));

  while (is_empty(work_queue) && !work_queue->stopped_accepting)
  {
    pthread_cond_wait(&work_queue->no_work_cv, &work_queue->mutex);
  }

  CHECKED(pthread_mutex_unlock(&work_queue->mutex));
}

err_t work_queue_add(work_queue_t * work_queue, const work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);

  err_t ret_err;
  
  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  ret_err = add_to_queue(work_queue, p_work);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return ret_err;
}

err_t work_queue_remove(work_queue_t * work_queue, work_t * p_work)
{
  assert(work_queue != NULL);
  assert(p_work != NULL);

  err_t ret_err;

  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  ret_err = remove_from_queue(work_queue, p_work);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return ret_err;
}

void work_queue_stop_accepting(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  CHECKED(pthread_mutex_lock(&work_queue->mutex));

  assert(!work_queue->stopped_accepting);

  work_queue->stopped_accepting = true;

  pthread_cond_broadcast(&work_queue->no_work_cv);

  CHECKED(pthread_mutex_unlock(&work_queue->mutex));
}

