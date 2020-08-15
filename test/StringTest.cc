#include "gtest/gtest.h"
#include "SDString.h"

TEST(StringTest, format) {
	Common::SDString s = Common::SDString::Format("%s %d %lld", "1234567890", 1234567890, 9876543210);
	EXPECT_STREQ(*s, "1234567890 1234567890 9876543210");
	std::printf("%s\n", *s);
}

TEST(StringTest, renew_format) {
	i8 buf[1200];
	std::memset(buf, 'b', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = 0;
	Common::SDString s = Common::SDString::Format("%s", buf);
	EXPECT_STREQ(*s, buf);
	std::printf("%s\n", *s);
}

TEST(StringTest, empty_trim) {
	Common::SDString s;
	s.TrimStartAndEndInline();
	EXPECT_EQ(s.Empty(), true);
}

TEST(StringTest, space_trim) {
	Common::SDString s("                ");
	s.TrimStartAndEndInline();
	EXPECT_STREQ(*s, "");
	s = "  \t  \t\t\t\r  \r\r\r  \n\n\n  \n\n  ";
	s.TrimStartAndEndInline();
	EXPECT_STREQ(*s, "");
}

TEST(StringTest, left_trim) {
	Common::SDString s("   \t   \r\n  left");
	s.TrimStartAndEndInline();
	EXPECT_STREQ(*s, "left");
}

TEST(StringTest, right_trim) {
	Common::SDString s("right   \t   \r\n  ");
	s.TrimStartAndEndInline();
	EXPECT_STREQ(*s, "right");
}

TEST(StringTest, trim) {
	Common::SDString s("   \t   \r\n  left    right   \t   \r\n  ");
	s.TrimStartAndEndInline();
	EXPECT_STREQ(*s, "left    right");
}