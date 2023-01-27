#include "gtest/gtest.h"

extern "C"
{
  #include "work_queue.h"
}

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

#define DUMMY_WORK                     \
{ .routine = dummy_work_routine,       \
  .arg = (void *) dummy_work_routine }

static const work_t dummy_work = DUMMY_WORK;

TEST(WorkQueue, creates_empty_queue)
{
  work_queue_t * queue = work_queue_create(3);
  
  EXPECT_TRUE(work_queue_is_empty(queue));

  work_queue_destroy(queue);
}

TEST(WorkQueue, add_and_remove_by_single_element)
{
  work_t temp;

  work_queue_t * queue = work_queue_create(3);

  for (size_t i = 0; i < 7; i++)
  {
    work_queue_add(queue, &dummy_work);
    work_queue_remove(queue, &temp);

    EXPECT_EQ(dummy_work, temp);
  }

  work_queue_destroy(queue);
}

TEST(WorkQueue, add_and_remove_by_many_elements)
{
  work_t temp;
  const work_t work = DUMMY_WORK;

  work_queue_t * queue = work_queue_create(10);

  for (size_t tries = 0; tries < 5; tries++)
  {
    for (size_t i = 0; i < 3; i++)
    {
      work_queue_add(queue, &work);
    }

    for (size_t i = 0; i < 3; i++)
    {
      work_queue_remove(queue, &temp);
      EXPECT_EQ(work, temp);
    }
  }

  work_queue_destroy(queue);
}

TEST(WorkQueue, add_and_remove_by_max_possible_elements)
{
  work_t temp;
  const work_t work = DUMMY_WORK;

  work_queue_t * queue = work_queue_create(10);

  for (size_t tries = 0; tries < 5; tries++)
  {
    for (size_t i = 0; i < 10; i++)
    {
      work_queue_add(queue, &work);
    }

    for (size_t i = 0; i < 10; i++)
    {
      work_queue_remove(queue, &temp);
      EXPECT_EQ(work, temp);
    }
  }

  work_queue_destroy(queue);
}

TEST(WorkQueue, removing_not_all_preserves_fifo)
{
  const size_t INITIALLY_ELEMS_NO = 7;

  work_t temp;

  work_queue_t * queue = work_queue_create(10);

  // "not all" condition
  for (size_t i = 0; i < INITIALLY_ELEMS_NO; i++)
  {
    work_queue_add(queue, &dummy_work);
  }

  // checking fifo
  for (size_t i = 0; i < 3; i++)
  {
    work_queue_add(queue, &dummy_work);
    work_queue_add(queue, &dummy_work);
    work_queue_add(queue, &dummy_work);

    work_queue_remove(queue, &temp);
    EXPECT_EQ(dummy_work, temp);

    work_queue_remove(queue, &temp);
    EXPECT_EQ(dummy_work, temp);

    work_queue_remove(queue, &temp);
    EXPECT_EQ(dummy_work, temp);
  }

  // check initials in the end
  for (size_t i = 0; i < INITIALLY_ELEMS_NO; i++)
  {
    work_queue_remove(queue, &temp);
    EXPECT_EQ(dummy_work, temp);
  }

  work_queue_destroy(queue);
}

