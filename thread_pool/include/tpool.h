#ifndef TPOOL_H
#define TPOOL_H

#include <stddef.h>

typedef struct tpool_s tpool_t;

typedef enum tpool_ret_e
{
  TPOOL_SUCCESS = 0,
  TPOOL_ESYSFAIL,
  TPOOL_EMEMALLOC,
  TPOOL_EREQREJECTED,
} tpool_ret_t;

typedef void (* tpool_work_routine_t)(void * context);

/**
 * @brief         Creates a thread pool.
 *
 * @param[out]    p_tpool
 * @param[in]     threads_number
 *
 * @retval        TPOOL_SUCCESS    Instance is created successfully.
 * @retval        TPOOL_ESYSFAIL   Threads could not be started.
 * @retval        TPOOL_EMEMALLOC  Failed to allocate memory.
 */
tpool_ret_t tpool_create(tpool_t ** p_tpool, size_t threads_number);

/**
 * @brief         Destroies a thread pool.
 *
 * @param[in]     tpool
 */
void tpool_destroy(tpool_t * tpool);

/**
 * @brief         Enqueues a new work to the internal work queue.
 *
 * @param[in]     tpool    Instance to enqueue the work.
 * @param[in]     routine  Work routine to be executed.
 * @param[in]     arg      Argument to be passed to the routine.
 *
 * @retval        TPOOL_SUCCESS       Instance is created successfully.
 * @retval        TPOOL_EMEMALLOC     Failed to allocate memory.
 * @retval        TPOOL_EREQREJECTED  No longer accepts new works.
 */
tpool_ret_t tpool_add_work(tpool_t * tpool, tpool_work_routine_t routine, void * arg);

/**
 * @brief         Stops accepting new works
 *
 * @param[in]     tpool
 */
void tpool_shutdown(tpool_t * tpool);

/**
 * @brief         Joins all threads in the pool.
 *
 * @note          The thread will not be unblocked, if the pool was not shutdown.
 *
 * @param[in]     tpool
 */
void tpool_join(tpool_t * tpool);

/**
 * @brief         Joins all threads in the pool, then destroies it.
 *
 * @note          The thread will not be unblocked, if the pool was not shutdown.
 *
 * @param[in]     tpool
 */
void tpool_join_then_destroy(tpool_t * tpool);

#endif

