#include "gtest/gtest.h"

extern "C"
{
  #include "fifo/fifo.h"
}

TEST(FIFO, creates_and_destroies)
{
  fifo_t * fifo = NULL;
  fifo_ret_t ret;
  
  ret = fifo_create_for_object_size(&fifo, sizeof(uint32_t));
  EXPECT_EQ(ret, FIFO_SUCCESS);

  EXPECT_NE(fifo, (void *) NULL);

  if (fifo != NULL) // blocked, because fifo_destroy() asserts on NULL
  {
    ret = fifo_destroy(fifo);
    EXPECT_EQ(ret, FIFO_SUCCESS);
  }
}

TEST(FIFO, creates_empty)
{
  fifo_t * fifo = NULL;
  
  ASSERT_EQ(fifo_create_for_object_size(&fifo, sizeof(uint32_t)), FIFO_SUCCESS);

  ASSERT_NE(fifo, (void *) NULL);

  EXPECT_TRUE(fifo_is_empty(fifo));

  ASSERT_EQ(fifo_destroy(fifo), FIFO_SUCCESS);
}

TEST(FIFO_Debug, asserts_on_dequeueing_from_empty)
{
  fifo_t * fifo = NULL;

  uint32_t result;
  
  ASSERT_EQ(fifo_create_for_object_size(&fifo, sizeof(uint32_t)), FIFO_SUCCESS);

  ASSERT_NE(fifo, (void *) NULL);

  ASSERT_TRUE(fifo_is_empty(fifo));

  EXPECT_DEBUG_DEATH(fifo_dequeue(fifo, &result), "");

  ASSERT_EQ(fifo_destroy(fifo), FIFO_SUCCESS);
}

TEST(FIFO, dequeues_the_only_element_once)
{
  fifo_t * fifo = NULL;
  fifo_ret_t ret;

  const uint32_t object = 42;
  uint32_t returned = 0;
  
  ASSERT_EQ(fifo_create_for_object_size(&fifo, sizeof(uint32_t)), FIFO_SUCCESS);

  ASSERT_NE(fifo, (void *) NULL);

  ret = fifo_enqueue(fifo, &object);
  EXPECT_EQ(ret, FIFO_SUCCESS);

  ret = fifo_dequeue(fifo, &returned);
  EXPECT_EQ(ret, FIFO_SUCCESS);

  EXPECT_EQ(object, returned);

  ASSERT_EQ(fifo_destroy(fifo), FIFO_SUCCESS);
}

TEST(FIFO, dequeues_the_only_element_repeated)
{
  fifo_t * fifo = NULL;
  fifo_ret_t ret;

  const uint32_t object = 42;
  uint32_t returned = 0;
  
  ASSERT_EQ(fifo_create_for_object_size(&fifo, sizeof(uint32_t)), FIFO_SUCCESS);

  ASSERT_NE(fifo, (void *) NULL);

  for (uint32_t i = 0; i < 10; i++)
  {
    ret = fifo_enqueue(fifo, &object);
    EXPECT_EQ(ret, FIFO_SUCCESS);

    ret = fifo_dequeue(fifo, &returned);
    EXPECT_EQ(ret, FIFO_SUCCESS);
  }

  EXPECT_EQ(object, returned);

  ASSERT_EQ(fifo_destroy(fifo), FIFO_SUCCESS);
}

TEST(FIFO, dequeue_bunch_of_numbers_in_same_order)
{
  fifo_t * fifo = NULL;
  fifo_ret_t ret;

  ASSERT_EQ(fifo_create_for_object_size(&fifo, sizeof(uint32_t)), FIFO_SUCCESS);

  ASSERT_NE(fifo, (void *) NULL);

  for (uint32_t object = 0; object < 5; object++)
  {
    ret = fifo_enqueue(fifo, &object);
    EXPECT_EQ(ret, FIFO_SUCCESS);
  }

  for (uint32_t object = 0; object < 5; object++)
  {
    uint32_t returned = -1;
    
    ret = fifo_dequeue(fifo, &returned);
    EXPECT_EQ(ret, FIFO_SUCCESS);

    EXPECT_EQ(returned, object);
  }

  ASSERT_EQ(fifo_destroy(fifo), FIFO_SUCCESS);
}

TEST(FIFO, preserves_fifo_after_dequeueing_not_all)
{
  fifo_t * fifo = NULL;
  fifo_ret_t ret;

  uint32_t head = 0; // incremented on enqueue
  uint32_t tail = 0; // incremented on dequeue

  ASSERT_EQ(fifo_create_for_object_size(&fifo, sizeof(uint32_t)), FIFO_SUCCESS);

  ASSERT_NE(fifo, (void *) NULL);

  for (; head < 10; head++)
  {
    ret = fifo_enqueue(fifo, &head);
    EXPECT_EQ(ret, FIFO_SUCCESS);
  }

  for (; head < 20; head++)
  {
    uint32_t returned = -1;

    ret = fifo_enqueue(fifo, &head);
    EXPECT_EQ(ret, FIFO_SUCCESS);

    ret = fifo_dequeue(fifo, &returned);
    EXPECT_EQ(ret, FIFO_SUCCESS);

    EXPECT_EQ(returned, tail++);
  }

  for (; tail < head; tail++)
  {
    uint32_t returned = -1;

    ret = fifo_dequeue(fifo, &returned);
    EXPECT_EQ(ret, FIFO_SUCCESS);

    EXPECT_EQ(returned, tail);
  }

  ASSERT_EQ(fifo_destroy(fifo), FIFO_SUCCESS);
}

