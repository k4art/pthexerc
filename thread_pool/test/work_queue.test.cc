#include "gtest/gtest.h"

#include <thread>

extern "C"
{
  #include "work_queue.h"
}

/******************************************************/

bool operator == (const work_t & lhs, const work_t & rhs)
{
  bool same_routine = lhs.routine == rhs.routine;
  bool same_arg     = lhs.arg == rhs.arg;

  return same_routine && same_arg;
}

static void dummy_work_routine(void * arg)
{
  // nothing
}

class WorkQueue : public testing::Test
{
protected:
  static const size_t DUMMIES_NUMBER = 1024;

  static work_t dummies_pool[DUMMIES_NUMBER];
  
protected:
  void SetUp() override
  {
    for (size_t i = 0; i < DUMMIES_NUMBER; i++)
    {
      /* Make works different using `arg` field */

      dummies_pool[i].routine = dummy_work_routine;
      dummies_pool[i].arg     = (void *) i;
    }
  }

  static const work_t * DummyWork(size_t n)
  {
    return &dummies_pool[n % DUMMIES_NUMBER];
  }
};

work_t WorkQueue::dummies_pool[DUMMIES_NUMBER];

/******************************************************/


TEST_F(WorkQueue, creates_empty_queue)
{
  work_queue_t * queue = work_queue_create();
  
  ASSERT_NE(queue, nullptr);

  EXPECT_TRUE(work_queue_is_empty(queue));

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, add_and_remove_by_single_element)
{
  work_t temp;

  work_queue_t * queue = work_queue_create();
  
  ASSERT_NE(queue, nullptr);

  for (size_t i = 0; i < 7; i++)
  {
    EXPECT_EQ(work_queue_push(queue, DummyWork(i)), SUCCESS);
    EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);

    EXPECT_EQ(*DummyWork(i), temp);
  }

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, add_and_remove_by_many_elements)
{
  work_t temp;

  work_queue_t * queue = work_queue_create();
  
  ASSERT_NE(queue, nullptr);

  for (size_t tries = 0; tries < 5; tries++)
  {
    for (size_t i = 0; i < 3; i++)
    {
      EXPECT_EQ(work_queue_push(queue, DummyWork(tries * 10 + i)), SUCCESS);
    }

    for (size_t i = 0; i < 3; i++)
    {
      EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
      EXPECT_EQ(*DummyWork(tries * 10 + i), temp);
    }
  }

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, removing_not_all_preserves_fifo)
{
  const size_t INITIALLY_ELEMS_NO = 7;

  size_t head = 0, tail = 0;

  work_t temp;

  work_queue_t * queue = work_queue_create();
  
  ASSERT_NE(queue, nullptr);

  for (size_t i = 0; i < INITIALLY_ELEMS_NO; i++)
  {
    ASSERT_EQ(work_queue_push(queue, DummyWork(head++)), SUCCESS);
  }

  for (size_t i = 0; i < 3; i++)
  {
    EXPECT_EQ(work_queue_push(queue, DummyWork(head++)), SUCCESS);
    EXPECT_EQ(work_queue_push(queue, DummyWork(head++)), SUCCESS);
    EXPECT_EQ(work_queue_push(queue, DummyWork(head++)), SUCCESS);

    EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
    EXPECT_EQ(*DummyWork(tail++), temp);

    EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
    EXPECT_EQ(*DummyWork(tail++), temp);

    EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
    EXPECT_EQ(*DummyWork(tail++), temp);
  }

  for (size_t i = 0; i < INITIALLY_ELEMS_NO; i++)
  {
    EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
    EXPECT_EQ(*DummyWork(tail++), temp);
  }

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, broadcasts_after_adding_to_empty)
{
  bool success_flag = false;

  work_queue_t * queue = work_queue_create();
  
  ASSERT_NE(queue, nullptr);

  ASSERT_EQ(work_queue_push(queue, DummyWork(0)), SUCCESS);

  std::thread thread_remover([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    work_t temp;
    EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
  });

  std::thread thread_waiter([&]() {
    work_queue_stop_accepting(queue);

    success_flag = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(15));

  EXPECT_TRUE(success_flag);

  thread_remover.detach();
  thread_waiter.detach();

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, stops_accepting_new_works)
{
  work_queue_t * queue = work_queue_create();

  ASSERT_NE(queue, nullptr);

  work_queue_stop_accepting(queue);

  EXPECT_EQ(work_queue_push(queue, DummyWork(0)), ERROR_OUT_OF_SERVICE);
  EXPECT_EQ(work_queue_push(queue, DummyWork(0)), ERROR_OUT_OF_SERVICE);

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, returns_works_before_stop_accepting)
{
  work_t temp;
  work_queue_t * queue = work_queue_create();

  ASSERT_NE(queue, nullptr);

  ASSERT_EQ(work_queue_push(queue, DummyWork(0)), SUCCESS);
  ASSERT_EQ(work_queue_push(queue, DummyWork(1)), SUCCESS);

  work_queue_stop_accepting(queue);

  EXPECT_EQ(work_queue_push(queue, DummyWork(2)), ERROR_OUT_OF_SERVICE);
  EXPECT_EQ(work_queue_push(queue, DummyWork(3)), ERROR_OUT_OF_SERVICE);

  EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
  EXPECT_EQ(temp, *DummyWork(0));

  EXPECT_EQ(work_queue_pop(queue, &temp), SUCCESS);
  EXPECT_EQ(temp, *DummyWork(1));

  work_queue_destroy(queue);
}

