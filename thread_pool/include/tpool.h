#ifndef TPOOL_H
#define TPOOL_H

#include <stddef.h>

typedef struct tpool_s tpool_t;

typedef enum tpool_ret_e
{
  TPOOL_SUCCESS      = 0,
  TPOOL_ESYSFAIL     = 1,
  TPOOL_EMEMALLOC    = 2,
  TPOOL_EREQREJECTED = 3,
  TPOOL_EINVARG,
} tpool_ret_t;

typedef void (* tpool_work_routine_t)(void * context);

/**
 * @brief         Creates a thread pool.
 *
 * @param[out]    p_tpool
 * @param[in]     threads_number  Should be at least 1.
 *
 * @retval        TPOOL_SUCCESS    Instance is created successfully.
 * @retval        TPOOL_EINVARG    Invalid arguments.
 * @retval        TPOOL_ESYSFAIL   Threads could not be started.
 * @retval        TPOOL_EMEMALLOC  Failed to allocate memory.
 */
tpool_ret_t tpool_create(tpool_t ** p_tpool, size_t threads_number);

/**
 * @brief         Destroys a thread pool.
 *
 * @param[in]     tpool
 *
 * @retval        TPOOL_SUCCESS  Operation succeed.
 * @retval        TPOOL_EINVARG  Invalid arguments.
 */
tpool_ret_t tpool_destroy(tpool_t * tpool);

/**
 * @brief         Enqueues a new work to the internal work queue.
 *
 * @param[in]     tpool    Instance to enqueue the work.
 * @param[in]     routine  Work routine to be executed.
 * @param[in]     arg      Argument to be passed to the routine.
 *
 * @retval        TPOOL_SUCCESS       Operation succeed.
 * @retval        TPOOL_EINVARG       Invalid arguments.
 * @retval        TPOOL_EMEMALLOC     Failed to allocate memory.
 * @retval        TPOOL_EREQREJECTED  No longer accepts new works.
 * @retval        TPOOL_ESYSFAIL      System prevented from success.
 */
tpool_ret_t tpool_add_work(tpool_t * tpool, tpool_work_routine_t routine, void * arg);

/**
 * @brief         Stops accepting new works.
 *
 * @note          Repeated calls have no effects.
 *
 * @param[in]     tpool
 *
 * @retval        TPOOL_SUCCESS   Operation succeed.
 * @retval        TPOOL_EINVARG   Invalid arguments.
 * @retval        TPOOL_ESYSFAIL  System prevented from success.
 */
tpool_ret_t tpool_shutdown(tpool_t * tpool);

/**
 * @brief         Joins all threads in the pool.
 *
 * @note          The thread will not be unblocked, if the pool was not shutdown.
 *
 * @param[in]     tpool
 *
 * @retval        TPOOL_SUCCESS   Operation succeed.
 * @retval        TPOOL_EINVARG   Invalid arguments.
 * @retval        TPOOL_ESYSFAIL  Some threads could not join.
 */
tpool_ret_t tpool_join(tpool_t * tpool);

/**
 * @brief         Joins all threads in the pool, then destroys it.
 *
 * @note          The thread will not be unblocked, if the pool was not shutdown.
 *
 * @param[in]     tpool
 *
 * @retval        TPOOL_SUCCESS   Operation succeed.
 * @retval        TPOOL_EINVARG   Invalid arguments.
 * @retval        TPOOL_ESYSFAIL  Some threads could not join.
 *                                In this case tpool was not destroyed.
 */
tpool_ret_t tpool_join_then_destroy(tpool_t * tpool);

#endif

