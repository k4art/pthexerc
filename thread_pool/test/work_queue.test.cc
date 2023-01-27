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

TEST(WorkQueue, add_and_remove_by_single_element)
{
  work_t temp;
  const work_t work = DUMMY_WORK;

  work_queue_t * queue = work_queue_create(3);

  for (size_t i = 0; i < 7; i++)
  {
    work_queue_add(queue, &work);
    work_queue_remove(queue, &temp);

    EXPECT_EQ(work, temp);
  }
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
}

