#ifndef ERRORS_THREAD_POOL_H
#define ERRORS_THREAD_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum err_e
{
  SUCCESS,
  ERROR_UNDERFLOW,
  ERROR_OVERFLOW,
  ERROR_OUT_OF_SERVICE,
} err_t;

#if defined(DEBUG)

#define CHECK_ERROR(error, action_text)              \
  do {                                               \
    if (error != 0)                                  \
    {                                                \
      fprintf(stderr, "[ERROR] %s\n", action_text);  \
      fprintf(stderr, "        at \"%s\":%d: %s\n",  \
              __FILE__, __LINE__, strerror(error));  \
      abort();                                       \
    }                                                \
  } while (0)

#else

#define CHECK_ERROR(error, action_text)             \
  do {                                              \
    if (error != 0)                                 \
    {                                               \
      fprintf(stderr, "[ERROR] %s\n", action_text); \
      abort();                                      \
    }                                               \
  } while (0)


#endif

#define CHECKED(statement)                         \
  do {                                             \
    int err = statement;                           \
    CHECK_ERROR(err, #statement);                  \
  } while (0)

#endif
