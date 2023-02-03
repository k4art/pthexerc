#ifndef TPOOL_COMMON_H
#define TPOOL_COMMON_H

#include <assert.h>

typedef enum err_e
{
  SUCCESS,
  ERROR_UNDERFLOW,
  ERROR_OVERFLOW,
  ERROR_OUT_OF_SERVICE,
} err_t;

#define asserting(expr) do {   \
    int ret = (expr);          \
    assert(ret && #expr); \
  } while(0)

#define TRY_EOK(n, expr)                 \
  do {                                   \
    if ((expr) != 0)                     \
      goto try_failure_ ## n;            \
  } while (0)

#define TRY_NEW(n, expr)                 \
  do {                                   \
    if ((expr) == NULL)                  \
      goto try_failure_ ## n;            \
  } while (0)


#endif

