#include "include/ringbuffer.h"

#include <gtest/gtest.h>

TEST(RingbufferTest, TestEmpty) {
  jellED::Ringbuffer ringbuffer(3);

  EXPECT_EQ(ringbuffer.get(0), 0.0);
  EXPECT_EQ(ringbuffer.get(1), 0.0);
  EXPECT_EQ(ringbuffer.get(2), 0.0);
}

TEST(RingbufferTest, TestInsert) {
  jellED::Ringbuffer ringbuffer(5);
  ringbuffer.append(1.0);
  ringbuffer.append(2.0);
  ringbuffer.append(3.0);
  ringbuffer.append(4.0);
  ringbuffer.append(5.0);
  ringbuffer.append(6.0);
  EXPECT_EQ(ringbuffer.get(0), 2.0);
  EXPECT_EQ(ringbuffer.get(1), 3.0);
  EXPECT_EQ(ringbuffer.get(2), 4.0);
  EXPECT_EQ(ringbuffer.get(3), 5.0);
  EXPECT_EQ(ringbuffer.get(4), 6.0);
}

TEST(RingbufferTest, TestGetOutOfBounds) {
  jellED::Ringbuffer ringbuffer(2);
  ringbuffer.append(1.0);
  ringbuffer.append(2.0);
  EXPECT_EQ(ringbuffer.get(0), 1.0);
  EXPECT_EQ(ringbuffer.get(1), 2.0);
  EXPECT_THROW(
      {
        try {
          ringbuffer.get(2);
        } catch (const std::invalid_argument &e) {
          EXPECT_STREQ(e.what(), "Index out of bounds for Ringbuffer get");
          throw;
        }
      },
      std::invalid_argument);
}
