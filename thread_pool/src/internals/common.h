#ifndef TPOOL_COMMON_H
#define TPOOL_COMMON_H

#include <assert.h>

typedef enum err_e
{
  /* these errors can be casted to tpool_ret_t */
  E_OK        = 0,
  E_SYSFAIL   = 1,
  E_MEMALLOC  = 2,
  E_BADREQ    = 3,

  /* these errors should not be casted to tpool_ret_t */
  E_UNDERFLOW = -1,
  E_OVERFLOW  = -2,
} err_t;

/**
 * Side-effects are allowed.
 */
#define asserting(expr)             \
  do {                              \
    int ret = (expr);               \
    assert(ret && #expr);           \
  } while(0)

/**
 * Side-effects are allowed.
 */
#define asserting_eok(expr)         \
   asserting((expr) == 0)

/**
 * "Goto"s to `try_failure_<n>`.
 */
#define TRY_EOK(n, expr)            \
  do {                              \
    if ((expr) != 0)                \
      goto try_failure_ ## n;       \
  } while (0)

/**
 * "Goto"s to `try_failure_<n>`.
 */
#define TRY_NEW(n, expr)            \
  do {                              \
    if ((expr) == NULL)             \
      goto try_failure_ ## n;       \
  } while (0)

#define TRUE_OR_RETURN(expr, ret)   \
  do {                              \
    if (!(expr))                    \
      return (ret);                 \
  } while (0)

#define EOK_OR_RETURN(expr, ret)    \
  do {                              \
    if ((expr) != 0)                \
      return (ret);                 \
  } while (0)

#define UNREACHABLE()                  \
  do {                                 \
    assert(!*"Reached UNREACHABLE()"); \
    __builtin_unreachable();           \
  } while (0);

/**
 * Locking a mutex asserts and returns E_SYSFAIL on failure.
 */
#define MUTEX_LOCK(p_mutex)                              \
  do {                                                   \
    int ret = pthread_mutex_lock(p_mutex);               \
    assert(ret == 0 && "pthread_mutex_lock() failed");   \
    EOK_OR_RETURN(ret, E_SYSFAIL);                       \
  } while (0)

/**
 * Unlocking a mutex asserts and returns E_SYSFAIL on failure.
 */
#define MUTEX_UNLOCK(p_mutex)                            \
  do {                                                   \
    int ret = pthread_mutex_unlock(p_mutex);             \
    assert(ret == 0 && "pthread_mutex_unlock() failed"); \
    EOK_OR_RETURN(ret, E_SYSFAIL);                       \
  } while (0)

#endif

