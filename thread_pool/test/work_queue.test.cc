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
  work_t temp;
  work_queue_t * queue = work_queue_create();
  
  ASSERT_NE(queue, nullptr);

  EXPECT_EQ(work_queue_pop(queue, &temp), E_UNDERFLOW);

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, add_and_remove_by_single_element)
{
  work_t temp;

  work_queue_t * queue = work_queue_create();
  
  ASSERT_NE(queue, nullptr);

  for (size_t i = 0; i < 7; i++)
  {
    EXPECT_EQ(work_queue_push(queue, DummyWork(i)), E_OK);
    EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);

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
      EXPECT_EQ(work_queue_push(queue, DummyWork(tries * 10 + i)), E_OK);
    }

    for (size_t i = 0; i < 3; i++)
    {
      EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);
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
    ASSERT_EQ(work_queue_push(queue, DummyWork(head++)), E_OK);
  }

  for (size_t i = 0; i < 3; i++)
  {
    EXPECT_EQ(work_queue_push(queue, DummyWork(head++)), E_OK);
    EXPECT_EQ(work_queue_push(queue, DummyWork(head++)), E_OK);
    EXPECT_EQ(work_queue_push(queue, DummyWork(head++)), E_OK);

    EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);
    EXPECT_EQ(*DummyWork(tail++), temp);

    EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);
    EXPECT_EQ(*DummyWork(tail++), temp);

    EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);
    EXPECT_EQ(*DummyWork(tail++), temp);
  }

  for (size_t i = 0; i < INITIALLY_ELEMS_NO; i++)
  {
    EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);
    EXPECT_EQ(*DummyWork(tail++), temp);
  }

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, stops_accepting_new_works)
{
  work_queue_t * queue = work_queue_create();

  ASSERT_NE(queue, nullptr);

  work_queue_stop_accepting(queue);

  EXPECT_EQ(work_queue_push(queue, DummyWork(0)), E_BADREQ);
  EXPECT_EQ(work_queue_push(queue, DummyWork(0)), E_BADREQ);

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, pushing_work_wakes_up)
{
  work_t temp;
  work_queue_t * queue = work_queue_create();
  bool wakeup_flag = false;

  ASSERT_NE(queue, nullptr);

  std::thread thread_waiter([&]() {
    work_queue_wait_while_no_work(queue);

    EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);

    wakeup_flag = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  EXPECT_EQ(work_queue_push(queue, DummyWork(0)), E_OK);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  thread_waiter.detach();

  EXPECT_TRUE(wakeup_flag);

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, stop_accepting_wakes_up)
{
  work_t temp;
  work_queue_t * queue = work_queue_create();
  bool wakeup_flag = false;

  ASSERT_NE(queue, nullptr);

  std::thread thread_waiter([&]() {
    work_queue_wait_while_no_work(queue);
    wakeup_flag = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  work_queue_stop_accepting(queue);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  thread_waiter.detach();

  EXPECT_TRUE(wakeup_flag);

  work_queue_destroy(queue);
}

TEST_F(WorkQueue, returns_works_before_stop_accepting)
{
  work_t temp;
  work_queue_t * queue = work_queue_create();

  ASSERT_NE(queue, nullptr);

  ASSERT_EQ(work_queue_push(queue, DummyWork(0)), E_OK);
  ASSERT_EQ(work_queue_push(queue, DummyWork(1)), E_OK);

  work_queue_stop_accepting(queue);

  EXPECT_EQ(work_queue_push(queue, DummyWork(2)), E_BADREQ);
  EXPECT_EQ(work_queue_push(queue, DummyWork(3)), E_BADREQ);

  EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);
  EXPECT_EQ(temp, *DummyWork(0));

  EXPECT_EQ(work_queue_pop(queue, &temp), E_OK);
  EXPECT_EQ(temp, *DummyWork(1));

  work_queue_destroy(queue);
}

