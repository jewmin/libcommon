#include "gtest/gtest.h"
#include "LocalTime.h"

TEST(LocalTimeTest, use) {
	std::time_t t = 1577808000;
	std::tm tm;
	Logger::LocalTime(&t, &tm);
	EXPECT_EQ(tm.tm_year, 120);
	EXPECT_EQ(tm.tm_mon, 0);
	EXPECT_EQ(tm.tm_mday, 1);
	EXPECT_EQ(tm.tm_hour, 0);
	EXPECT_EQ(tm.tm_min, 0);
	EXPECT_EQ(tm.tm_sec, 0);
}