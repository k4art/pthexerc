#ifndef TPOOL_H
#define TPOOL_H

typedef struct tpool_s tpool_t;

tpool_t * tpool_create(size_t threads_number);
void tpool_destroy(tpool_t * tpool);

void tpool_wait(tpool_t * tpool);

#endif

