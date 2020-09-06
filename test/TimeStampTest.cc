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