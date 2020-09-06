#include "gtest/gtest.h"
#include "LocalTime.h"

TEST(LocalTimeTest, use) {
	std::time_t t = 1580704496;
	std::tm tm;
	jc_localtime(&t, &tm);
	EXPECT_EQ(tm.tm_year, 120);
	EXPECT_EQ(tm.tm_mon, 1);
	EXPECT_EQ(tm.tm_mday, 3);
	EXPECT_EQ(tm.tm_hour, 12);
	EXPECT_EQ(tm.tm_min, 34);
	EXPECT_EQ(tm.tm_sec, 56);
}