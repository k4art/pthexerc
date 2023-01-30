#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "errors.h"

#include "work_queue.h"

typedef struct queue_node_s queue_node_t;

struct queue_node_s
{
  work_t         work;
  queue_node_t * next;
  queue_node_t * prev;
};

struct work_queue_s
{
  queue_node_t    * head;
  queue_node_t    * tail;

  pthread_mutex_t mutex;
  pthread_cond_t  no_work_cv;

  bool stopped_accepting;
};

static size_t inc_mod(size_t n, size_t mod)
{
  assert(mod != 0);

  return (n + 1) % mod;
}

static void * malloc_c(size_t size)
{
  void * memory = malloc(size);

  if (memory == NULL)
  {
    fprintf(stderr, "malloc() returned NULL.\n");
    fprintf(stderr, "exiting...");

    exit(EXIT_FAILURE);
  }

  return memory;
}

static bool is_empty(work_queue_t * work_queue)
{
  bool has_no_elements = work_queue->head == NULL;

  /* head == NULL iff tail == NULL */
  assert((work_queue->head == NULL) == (work_queue->tail == NULL));

  return has_no_elements;
}

static err_t add_to_queue(work_queue_t * work_queue, const work_t * p_work)
{
  if (work_queue->stopped_accepting)
  {
    return ERROR_OUT_OF_SERVICE;
  }

  queue_node_t * queue_node = malloc_c(sizeof(queue_node_t));

  queue_node->work = *p_work;
  queue_node->prev = work_queue->tail;
  queue_node->next = NULL;

  if (work_queue->tail != NULL)
  {
    work_queue->tail->next = queue_node;
  }

  if (work_queue->head == NULL) /* if it happens to be the first element as well */
  {
    work_queue->head = queue_node;
  }

  work_queue->tail = queue_node;
  
  return SUCCESS;
}

static err_t remove_from_queue(work_queue_t * work_queue, work_t * p_work)
{
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

  queue_node_t * pre_tail = work_queue->tail->prev;

  memcpy(p_work, &work_queue->tail->work, sizeof(*p_work));

  free(work_queue->tail);

  if (pre_tail != NULL)
  {
    pre_tail->next = NULL;
    work_queue->tail = pre_tail;
  }
  else
  {
    work_queue->tail = NULL;
    work_queue->head = NULL;
  }

  return SUCCESS;
}

work_queue_t * work_queue_create(void)
{
  
  work_queue_t * queue = malloc_c(sizeof(work_queue_t));

  queue->head = NULL;
  queue->tail = NULL;

  CHECKED(pthread_mutex_init(&queue->mutex, NULL));
  CHECKED(pthread_cond_init(&queue->no_work_cv, NULL));

  queue->stopped_accepting = false;

  assert(is_empty(queue));
  
  return queue;
}

void work_queue_destroy(work_queue_t * work_queue)
{
  assert(work_queue != NULL);

  queue_node_t * node = work_queue->head;
  queue_node_t * next = NULL;

  while (node != NULL)
  {
    next = node->next;
    free(node);
    node = next;
  }
  
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

  while (is_empty(work_queue) || !work_queue->stopped_accepting)
  {
    pthread_cond_wait(&work_queue->no_work_cv, &work_queue->mutex);
  }

  CHECKED(pthread_mutex_unlock(&work_queue->mutex));
}

err_t work_queue_add(work_queue_t * work_queue, const work_t * p_work)
{
  assert(work_queue != NULL);

  err_t ret_err;
  
  CHECKED(pthread_mutex_lock(&work_queue->mutex));
  ret_err = add_to_queue(work_queue, p_work);
  CHECKED(pthread_mutex_unlock(&work_queue->mutex));

  return ret_err;
}

err_t work_queue_remove(work_queue_t * work_queue, work_t * p_work)
{
  assert(work_queue != NULL);

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

