#include "gtest/gtest.h"
#include "SDString.h"

TEST(StringTest, hdr) {
	EXPECT_EQ(Common::SDString::HdrSize(0), 0);
	EXPECT_EQ(Common::SDString::HdrSize(SDS_TYPE_8), sizeof(struct Common::sdshdr8));
	EXPECT_EQ(Common::SDString::HdrSize(SDS_TYPE_16), sizeof(struct Common::sdshdr16));
	EXPECT_EQ(Common::SDString::HdrSize(SDS_TYPE_32), sizeof(struct Common::sdshdr32));
	EXPECT_EQ(Common::SDString::HdrSize(SDS_TYPE_64), sizeof(struct Common::sdshdr64));
}

TEST(StringTest, default_ctor) {
	Common::SDString s;
	EXPECT_STREQ(*s, "");
	EXPECT_EQ(s.Empty(), true);
	EXPECT_EQ(s.Size(), (size_t)0);
	EXPECT_EQ(s.AllocSize(), (size_t)0);
	EXPECT_EQ(s.AvailSize(), (size_t)0);
}

TEST(StringTest, char_ctor) {
	Common::SDString s(10, 'S');
	EXPECT_STREQ(*s, "SSSSSSSSSS");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), (size_t)10);
	EXPECT_EQ(s.AllocSize(), (size_t)10);
	EXPECT_EQ(s.AvailSize(), (size_t)0);
}

TEST(StringTest, string_ctor) {
	Common::SDString s("hello world");
	EXPECT_STREQ(*s, "hello world");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("hello world"));
	EXPECT_EQ(s.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s.AvailSize(), (size_t)0);
}

TEST(StringTest, copy_ctor) {
	Common::SDString s("hello world");
	Common::SDString s1(s);
	EXPECT_STREQ(*s, *s1);
	EXPECT_EQ(s.Empty(), s1.Empty());
	EXPECT_EQ(s.Size(), s1.Size());
	EXPECT_EQ(s.AllocSize(), s1.Size());
	EXPECT_EQ(s.AvailSize(), s1.AvailSize());
}

TEST(StringTest, copy_ctor_sub) {
	Common::SDString s("hello world");
	Common::SDString s1(s, 0, 5), s2(s, 6);
	EXPECT_STREQ(*s1, "hello");
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), (size_t)5);
	EXPECT_EQ(s1.AllocSize(), (size_t)5);
	EXPECT_EQ(s1.AvailSize(), (size_t)0);
	
	EXPECT_STREQ(*s2, "world");
	EXPECT_EQ(s2.Empty(), false);
	EXPECT_EQ(s2.Size(), (size_t)5);
	EXPECT_EQ(s2.AllocSize(), (size_t)5);
	EXPECT_EQ(s2.AvailSize(), (size_t)0);
}

TEST(StringTest, move_ctor) {
	Common::SDString s("hello world");
	Common::SDString s1(std::move(s));
	EXPECT_STREQ(*s1, "hello world");
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), std::strlen("hello world"));
	EXPECT_EQ(s1.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s1.AvailSize(), (size_t)0);
}

TEST(StringTest, assign_char_ctor) {
	Common::SDString s("hello world");
	s = 'M';
	EXPECT_STREQ(*s, "M");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), (size_t)1);
	EXPECT_EQ(s.AllocSize(), (size_t)11);
	EXPECT_EQ(s.AvailSize(), (size_t)10);
}

TEST(StringTest, assign_string_ctor) {
	Common::SDString s("hello world");
	s = "I'm a programmer.";
	EXPECT_STREQ(*s, "I'm a programmer.");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AllocSize(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AvailSize(), (size_t)0);
}

TEST(StringTest, assign_copy_ctor) {
	Common::SDString s("hello world");
	s = "I'm a programmer.";
	EXPECT_STREQ(*s, "I'm a programmer.");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AllocSize(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AvailSize(), (size_t)0);
}

TEST(StringTest, assign_move_ctor) {
	Common::SDString s("hello world");
	s = Common::SDString("you are welcome");
	EXPECT_STREQ(*s, "you are welcome");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("you are welcome"));
	EXPECT_EQ(s.AllocSize(), std::strlen("you are welcome"));
	EXPECT_EQ(s.AvailSize(), (size_t)0);
}

TEST(StringTest, new_op) {
	Common::SDString * s = new Common::SDString("hello world");
	EXPECT_STREQ(*(*s), "hello world");
	EXPECT_EQ(s->Empty(), false);
	EXPECT_EQ(s->Size(), std::strlen("hello world"));
	EXPECT_EQ(s->AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s->AvailSize(), (size_t)0);
	delete s;
}

TEST(StringTest, clear) {
	Common::SDString s("hello world");
	s.Clear();
	EXPECT_STREQ(*s, "");
	EXPECT_EQ(s.Empty(), true);
	EXPECT_EQ(s.Size(), (size_t)0);
	EXPECT_EQ(s.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s.AvailSize(), std::strlen("hello world"));
}

TEST(StringTest, remove) {
	Common::SDString s("hello world");
	s.RemoveAt(5);
	EXPECT_STREQ(*s, "helloworld");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("helloworld"));
	EXPECT_EQ(s.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s.AvailSize(), (size_t)1);

	s.RemoveAt(5, 5);
	EXPECT_STREQ(*s, "hello");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("hello"));
	EXPECT_EQ(s.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s.AvailSize(), std::strlen("hello world") - std::strlen("hello"));
}

TEST(StringTest, substr) {
	Common::SDString s("hello world");
	EXPECT_STREQ(*(s.SubStr(0, 5)), "hello");
	Common::SDString s1 = s.SubStr(6);
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), std::strlen("world"));
	EXPECT_EQ(s1.AllocSize(), std::strlen("world"));
	EXPECT_EQ(s1.AvailSize(), (size_t)0);
}

TEST(StringTest, assign_char) {
	
}

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