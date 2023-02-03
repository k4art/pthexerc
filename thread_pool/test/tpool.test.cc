#include "gtest/gtest.h"

extern "C"
{
  #include "tpool.h"
}

TEST(TPoolSingleThreaded, joins_when_no_works_given)
{
  tpool_t * tpool = NULL;
  
  ASSERT_EQ(tpool_create(&tpool, 1), TPOOL_SUCCESS);

  tpool_shutdown(tpool);
  tpool_join_then_destroy(tpool);
}

TEST(TPoolSingleThreaded, executes_all_works_in_fifo)
{
  tpool_t * tpool = NULL;

  ASSERT_EQ(tpool_create(&tpool, 1), TPOOL_SUCCESS);

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
    EXPECT_EQ(tpool_add_work(tpool, work_routine, (void *) (intptr_t) i), TPOOL_SUCCESS);
  }

  tpool_shutdown(tpool);
  tpool_join_then_destroy(tpool);
}

TEST(TPoolMultiThreaded, joins_when_no_works_given)
{
  tpool_t * tpool = NULL;

  ASSERT_EQ(tpool_create(&tpool, 8), TPOOL_SUCCESS);

  tpool_shutdown(tpool);
  tpool_join_then_destroy(tpool);
}

TEST(TPoolMultiThreaded, executes_all_works_multiple_of_threads_number)
{
  const size_t TOTAL_WORKS_NO = 8;

  bool work_done_f[TOTAL_WORKS_NO];
  
  tpool_t * tpool = NULL;

  ASSERT_EQ(tpool_create(&tpool, 8), TPOOL_SUCCESS);

  auto work_routine = [](void * context)
    {
      bool * flag = (bool *) context;

      *flag = true;
    };

  for (int i = 0; i < TOTAL_WORKS_NO; i++)
  {
    EXPECT_EQ(tpool_add_work(tpool, work_routine, (void *) &work_done_f[i]), TPOOL_SUCCESS);
  }

  tpool_shutdown(tpool);
  tpool_join(tpool);

  for (size_t i = 0; i < TOTAL_WORKS_NO; i++)
  {
    EXPECT_TRUE(work_done_f[i]);
  }

  tpool_destroy(tpool);
}

TEST(TPoolMultiThreaded, rejects_works_after_shutdown)
{
  tpool_t * tpool = NULL;

  auto routine = [](void *){};

  ASSERT_EQ(tpool_create(&tpool, 8), TPOOL_SUCCESS);

  tpool_shutdown(tpool);

  EXPECT_EQ(tpool_add_work(tpool, routine, NULL), TPOOL_EREQREJECTED);

  tpool_join_then_destroy(tpool);
}

