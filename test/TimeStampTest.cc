#include "gtest/gtest.h"
#include "TimeStamp.h"

TEST(TimeStampTest, default_ctor) {
	Common::TimeStamp s;
	std::printf("seconds: %lld, milliseconds: %lld, microseconds: %lld\n", s.GetSeconds(), s.GetMilliSeconds(), s.GetMicroSeconds());
}

TEST(TimeStampTest, args_ctor) {
	Common::TimeStamp s(1234, 567890);
	EXPECT_EQ(s.GetSeconds(), 1234);
	EXPECT_EQ(s.GetMilliSeconds(), 567);
	EXPECT_EQ(s.GetMicroSeconds(), 567890);
	std::printf("seconds: %lld, milliseconds: %lld, microseconds: %lld\n", s.GetSeconds(), s.GetMilliSeconds(), s.GetMicroSeconds());
}

TEST(TimeStampTest, copy_ctor) {
	Common::TimeStamp s;
	std::printf("seconds: %lld, milliseconds: %lld, microseconds: %lld\n", s.GetSeconds(), s.GetMilliSeconds(), s.GetMicroSeconds());
	Common::TimeStamp t(s);
	EXPECT_EQ(s.GetSeconds(), t.GetSeconds());
	EXPECT_EQ(s.GetMilliSeconds(), t.GetMilliSeconds());
	EXPECT_EQ(s.GetMicroSeconds(), t.GetMicroSeconds());
	std::printf("seconds: %lld, milliseconds: %lld, microseconds: %lld\n", t.GetSeconds(), t.GetMilliSeconds(), t.GetMicroSeconds());
}

TEST(TimeStampTest, assign_ctor) {
	Common::TimeStamp s(1234, 567890);
	std::printf("seconds: %lld, milliseconds: %lld, microseconds: %lld\n", s.GetSeconds(), s.GetMilliSeconds(), s.GetMicroSeconds());
	Common::TimeStamp t;
	std::printf("seconds: %lld, milliseconds: %lld, microseconds: %lld\n", t.GetSeconds(), t.GetMilliSeconds(), t.GetMicroSeconds());
	t = t;
	t = s;
	EXPECT_EQ(s.GetSeconds(), t.GetSeconds());
	EXPECT_EQ(s.GetMilliSeconds(), t.GetMilliSeconds());
	EXPECT_EQ(s.GetMicroSeconds(), t.GetMicroSeconds());
	std::printf("seconds: %lld, milliseconds: %lld, microseconds: %lld\n", t.GetSeconds(), t.GetMilliSeconds(), t.GetMicroSeconds());
}

TEST(TimeStampTest, eq_op) {
	Common::TimeStamp s1(1234, 567890), s2(1234, 567890);
	EXPECT_EQ(s1, s2);
	EXPECT_LE(s1, s2);
	EXPECT_GE(s1, s2);

	EXPECT_TRUE(s1 == s2);
	EXPECT_TRUE(s1 <= s2);
	EXPECT_TRUE(s1 >= s2);
}

TEST(TimeStampTest, ne_op) {
	Common::TimeStamp s1(1234, 567890), s2(1234, 567891), s3(1235, 567890);
	EXPECT_NE(s1, s2);
	EXPECT_NE(s2, s3);
	EXPECT_NE(s1, s3);

	EXPECT_TRUE(s1 != s2);
	EXPECT_TRUE(s2 != s3);
	EXPECT_TRUE(s1 != s3);
}

TEST(TimeStampTest, lt_op) {
	Common::TimeStamp s1(1234, 567890), s2(1234, 567891), s3(1235, 567890);
	EXPECT_LT(s1, s2);
	EXPECT_LT(s2, s3);
	EXPECT_LT(s1, s3);

	EXPECT_LE(s1, s2);
	EXPECT_LE(s2, s3);
	EXPECT_LE(s1, s3);

	EXPECT_TRUE(s1 < s2);
	EXPECT_TRUE(s2 < s3);
	EXPECT_TRUE(s1 < s3);

	EXPECT_TRUE(s1 <= s2);
	EXPECT_TRUE(s2 <= s3);
	EXPECT_TRUE(s1 <= s3);
}

TEST(TimeStampTest, gt_op) {
	Common::TimeStamp s1(1234, 567890), s2(1234, 567891), s3(1235, 567890);
	EXPECT_GT(s2, s1);
	EXPECT_GT(s3, s2);
	EXPECT_GT(s3, s1);

	EXPECT_GE(s2, s1);
	EXPECT_GE(s3, s2);
	EXPECT_GE(s3, s1);

	EXPECT_TRUE(s2 > s1);
	EXPECT_TRUE(s3 > s2);
	EXPECT_TRUE(s3 > s1);

	EXPECT_TRUE(s2 >= s1);
	EXPECT_TRUE(s3 >= s2);
	EXPECT_TRUE(s3 >= s1);
}