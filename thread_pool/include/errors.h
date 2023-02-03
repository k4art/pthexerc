#ifndef ERRORS_THREAD_POOL_H
#define ERRORS_THREAD_POOL_H

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

#endif

