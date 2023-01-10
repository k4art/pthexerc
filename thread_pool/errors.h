#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#endif
