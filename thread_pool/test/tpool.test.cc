#include "gtest/gtest.h"

extern "C"
{
  #include "tpool.h"
  #include "work_queue.h"
}

TEST(TPoolSingleThreaded, joins_when_no_works_given)
{
  tpool_t * tpool = tpool_create(1);

  tpool_shutdown(tpool);
  tpool_join_then_destroy(tpool);
}

TEST(TPoolSingleThreaded, executes_all_works_in_fifo)
{
  tpool_t * tpool = tpool_create(1);

  auto work_routine = [](void * context)
    {
      /* For single thread test case, for no syncrhonization needed */
      static int previous = -1;

      int current = (intptr_t) (context);

      EXPECT_EQ(current, previous + 1);

      previous = current;
    };

  for (int i = 0; i < 32; i++)
  {
    tpool_add_work(tpool, work_routine, (void *) (intptr_t) i);
  }

  tpool_shutdown(tpool);
  tpool_join_then_destroy(tpool);
}

TEST(TPoolMultiThreaded, joins_when_no_works_given)
{
  tpool_t * tpool = tpool_create(8);

  tpool_shutdown(tpool);
  tpool_join_then_destroy(tpool);
}

TEST(TPoolMultiThreaded, executes_all_works_multiple_of_threads_number)
{
  const size_t TOTAL_WORKS_NO = 8;
  bool work_done_f[TOTAL_WORKS_NO];
  
  tpool_t * tpool = tpool_create(8);

  auto work_routine = [](void * context)
    {
      bool * flag = (bool *) context;

      *flag = true;
    };

  for (int i = 0; i < TOTAL_WORKS_NO; i++)
  {
    tpool_add_work(tpool, work_routine, (void *) &work_done_f[i]);
  }

  tpool_shutdown(tpool);
  tpool_join(tpool);

  for (size_t i = 0; i < TOTAL_WORKS_NO; i++)
  {
    EXPECT_TRUE(work_done_f[i]);
  }

  tpool_destroy(tpool);
}

