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
	EXPECT_EQ(s.Size(), static_cast<size_t>(0));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(0));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, char_ctor) {
	Common::SDString s(10, 'S');
	EXPECT_STREQ(*s, "SSSSSSSSSS");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), static_cast<size_t>(10));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(10));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, string_ctor) {
	Common::SDString s("hello world");
	EXPECT_STREQ(*s, "hello world");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("hello world"));
	EXPECT_EQ(s.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
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
	EXPECT_EQ(s1.Size(), static_cast<size_t>(5));
	EXPECT_EQ(s1.AllocSize(), static_cast<size_t>(5));
	EXPECT_EQ(s1.AvailSize(), static_cast<size_t>(0));

	EXPECT_STREQ(*s2, "world");
	EXPECT_EQ(s2.Empty(), false);
	EXPECT_EQ(s2.Size(), static_cast<size_t>(5));
	EXPECT_EQ(s2.AllocSize(), static_cast<size_t>(5));
	EXPECT_EQ(s2.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, move_ctor) {
	Common::SDString s("hello world");
	Common::SDString s1(std::move(s));
	EXPECT_STREQ(*s1, "hello world");
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), std::strlen("hello world"));
	EXPECT_EQ(s1.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s1.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, assign_char_ctor) {
	Common::SDString s("hello world");
	s = 'M';
	EXPECT_STREQ(*s, "M");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), static_cast<size_t>(1));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(11));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(10));
}

TEST(StringTest, assign_string_ctor) {
	Common::SDString s("hello world");
	s = "I'm a programmer.";
	EXPECT_STREQ(*s, "I'm a programmer.");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AllocSize(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, assign_copy_ctor) {
	Common::SDString s("hello world");
	s = Common::SDString("I'm a programmer.");
	EXPECT_STREQ(*s, "I'm a programmer.");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AllocSize(), std::strlen("I'm a programmer."));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, assign_move_ctor) {
	Common::SDString s("hello world");
	s = std::move(Common::SDString("you are welcome"));
	EXPECT_STREQ(*s, "you are welcome");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("you are welcome"));
	EXPECT_EQ(s.AllocSize(), std::strlen("you are welcome"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, s16_ctor) {
	Common::SDString s(256, 'M');
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), static_cast<size_t>(256));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(256));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
	s.Clear();
}

TEST(StringTest, s32_ctor) {
	Common::SDString s(65536, 'S');
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), static_cast<size_t>(65536));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(65536));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
	s.Clear();
}

TEST(StringTest, new_op) {
	Common::SDString * s = new Common::SDString("hello world");
	EXPECT_STREQ(*(*s), "hello world");
	EXPECT_EQ(s->Empty(), false);
	EXPECT_EQ(s->Size(), std::strlen("hello world"));
	EXPECT_EQ(s->AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s->AvailSize(), static_cast<size_t>(0));
	delete s;
}

TEST(StringTest, clear) {
	Common::SDString s("hello world");
	s.Clear();
	EXPECT_STREQ(*s, "");
	EXPECT_EQ(s.Empty(), true);
	EXPECT_EQ(s.Size(), static_cast<size_t>(0));
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
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(1));

	s.RemoveAt(5, 5);
	EXPECT_STREQ(*s, "hello");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("hello"));
	EXPECT_EQ(s.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(6));
}

TEST(StringTest, substr) {
	Common::SDString s("hello world");
	EXPECT_STREQ(*(s.SubStr(0, 5)), "hello");
	Common::SDString s1 = s.SubStr(6);
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), std::strlen("world"));
	EXPECT_EQ(s1.AllocSize(), std::strlen("world"));
	EXPECT_EQ(s1.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, assign_char) {
	Common::SDString s;
	s.Assign(10, 'M');
	EXPECT_STREQ(*s, "MMMMMMMMMM");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), static_cast<size_t>(10));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(10));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, assign_string) {
	Common::SDString s;
	s.Assign("welcome to my code world");
	EXPECT_STREQ(*s, "welcome to my code world");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("welcome to my code world"));
	EXPECT_EQ(s.AllocSize(), std::strlen("welcome to my code world"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));

	Common::SDString s1;
	s1.Assign("welcome to my code world", 7);
	EXPECT_STREQ(*s1, "welcome");
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), std::strlen("welcome"));
	EXPECT_EQ(s1.AllocSize(), std::strlen("welcome"));
	EXPECT_EQ(s1.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, assign_copy) {
	Common::SDString s("hello world");
	Common::SDString s1;
	s1.Assign(s);
	EXPECT_STREQ(*s, *s1);
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), std::strlen("hello world"));
	EXPECT_EQ(s1.AllocSize(), std::strlen("hello world"));
	EXPECT_EQ(s1.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, assign) {
	Common::SDString s(100, 'M');
	s.Clear();
	s.Assign(5, '*');
	s.Assign("Welcome To My Code World");
	EXPECT_STREQ(*s, "Welcome To My Code World");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("Welcome To My Code World"));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(100));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(100) - std::strlen("Welcome To My Code World"));
}

TEST(StringTest, assign_copy_sub) {
	Common::SDString s("hello world");
	Common::SDString s1;
	s1.Assign(s, 6, 5);
	EXPECT_STREQ(*s1, "world");
	EXPECT_EQ(s1.Empty(), false);
	EXPECT_EQ(s1.Size(), std::strlen("world"));
	EXPECT_EQ(s1.AllocSize(), std::strlen("world"));
	EXPECT_EQ(s1.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, append) {
	Common::SDString s;
	EXPECT_STREQ(*s, "");
	EXPECT_EQ(s.Empty(), true);
	EXPECT_EQ(s.Size(), static_cast<size_t>(0));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(0));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));

	s.Append(5, 'A');
	EXPECT_STREQ(*s, "AAAAA");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("AAAAA"));
	EXPECT_EQ(s.AllocSize(), std::strlen("AAAAA"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));

	s.Append(" hello world");
	EXPECT_STREQ(*s, "AAAAA hello world");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("AAAAA hello world"));
	EXPECT_EQ(s.AllocSize(), std::strlen("AAAAA hello world"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));

	s.Append(" and big world", 8);
	EXPECT_STREQ(*s, "AAAAA hello world and big");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("AAAAA hello world and big"));
	EXPECT_EQ(s.AllocSize(), std::strlen("AAAAA hello world and big"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));

	s.Append(Common::SDString(" welcome to my code world"));
	EXPECT_STREQ(*s, "AAAAA hello world and big welcome to my code world");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("AAAAA hello world and big welcome to my code world"));
	EXPECT_EQ(s.AllocSize(), std::strlen("AAAAA hello world and big welcome to my code world"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));

	s.Append(Common::SDString(" happy new year"), 6, 4);
	EXPECT_STREQ(*s, "AAAAA hello world and big welcome to my code world new");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("AAAAA hello world and big welcome to my code world new"));
	EXPECT_EQ(s.AllocSize(), std::strlen("AAAAA hello world and big welcome to my code world new"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, add) {
	Common::SDString s = Common::SDString("I ") + Common::SDString("am ") + 'X' + "-Man";
	EXPECT_STREQ(*s, "I am X-Man");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("I am X-Man"));
	EXPECT_EQ(s.AllocSize(), std::strlen("I am X-Man"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, add_char) {
	Common::SDString s = Common::SDString("") + 'M';
	EXPECT_STREQ(*s, "M");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("M"));
	EXPECT_EQ(s.AllocSize(), std::strlen("M"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, add_string) {
	Common::SDString s = Common::SDString("") + "SuperMan" + "";
	EXPECT_STREQ(*s, "SuperMan");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("SuperMan"));
	EXPECT_EQ(s.AllocSize(), std::strlen("SuperMan"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, add_other) {
	Common::SDString s = Common::SDString("") + Common::SDString("Student") + Common::SDString("");
	EXPECT_STREQ(*s, "Student");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("Student"));
	EXPECT_EQ(s.AllocSize(), std::strlen("Student"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, add_assign) {
	Common::SDString s;
	s += Common::SDString("I am ");
	s += 'X';
	s += "-Man";
	EXPECT_STREQ(*s, "I am X-Man");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("I am X-Man"));
	EXPECT_EQ(s.AllocSize(), std::strlen("I am X-Man"));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(0));
}

TEST(StringTest, add_assign_avail) {
	Common::SDString s(100, 'M');
	s.Clear();
	s += Common::SDString("I am ");
	s += 'X';
	s += "-Man";
	EXPECT_STREQ(*s, "I am X-Man");
	EXPECT_EQ(s.Empty(), false);
	EXPECT_EQ(s.Size(), std::strlen("I am X-Man"));
	EXPECT_EQ(s.AllocSize(), static_cast<size_t>(100));
	EXPECT_EQ(s.AvailSize(), static_cast<size_t>(100) - std::strlen("I am X-Man"));
}

TEST(StringTest, at) {
	Common::SDString s("hello world");
	EXPECT_EQ(s[0], 'h');
	EXPECT_EQ(s[1], 'e');
	EXPECT_EQ(s[2], 'l');
	EXPECT_EQ(s[3], 'l');
	EXPECT_EQ(s[4], 'o');
	EXPECT_EQ(s[5], ' ');
	EXPECT_EQ(s[6], 'w');
	EXPECT_EQ(s[7], 'o');
	EXPECT_EQ(s[8], 'r');
	EXPECT_EQ(s[9], 'l');
	EXPECT_EQ(s[10], 'd');
}

void StringTest_AtConst(const Common::SDString & s) {
	EXPECT_EQ(s[0], 'h');
	EXPECT_EQ(s[1], 'e');
	EXPECT_EQ(s[2], 'l');
	EXPECT_EQ(s[3], 'l');
	EXPECT_EQ(s[4], 'o');
	EXPECT_EQ(s[5], ' ');
	EXPECT_EQ(s[6], 'w');
	EXPECT_EQ(s[7], 'o');
	EXPECT_EQ(s[8], 'r');
	EXPECT_EQ(s[9], 'l');
	EXPECT_EQ(s[10], 'd');
}

TEST(StringTest, at_const) {
	const Common::SDString * s = new Common::SDString("hello world");
	StringTest_AtConst(*s);
	delete s;
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

TEST(StringTest, trim_se) {
	Common::SDString s("   \t   \r\n  left    right   \t   \r\n  ");
	EXPECT_STREQ(*s.TrimStartAndEnd(), "left    right");
	EXPECT_STREQ(*Common::SDString("   \t   \r\n  left    right   \t   \r\n  ").TrimStartAndEnd(), "left    right");
}

TEST(StringTest, trim_s) {
	Common::SDString s("   \t   \r\n  left");
	EXPECT_STREQ(*s.TrimStart(), "left");
	EXPECT_STREQ(*Common::SDString("   \t   \r\n  left").TrimStart(), "left");
}

TEST(StringTest, trim_e) {
	Common::SDString s("right   \t   \r\n  ");
	EXPECT_STREQ(*s.TrimEnd(), "right");
	EXPECT_STREQ(*Common::SDString("right   \t   \r\n  ").TrimEnd(), "right");
}

TEST(StringTest, eq_op) {
	i8 buf[] = "1234567890";
	Common::SDString s("1234567890"), s1("1234567890");
	EXPECT_EQ(s, buf);
	EXPECT_EQ(s, s1);
	EXPECT_TRUE(s == "1234567890");
	EXPECT_TRUE(s == Common::SDString("1234567890"));

	EXPECT_LE(s, buf);
	EXPECT_LE(s, s1);
	EXPECT_TRUE(s <= "1234567890");
	EXPECT_TRUE(s <= Common::SDString("1234567890"));

	EXPECT_GE(s, buf);
	EXPECT_GE(s, s1);
	EXPECT_TRUE(s >= "1234567890");
	EXPECT_TRUE(s >= Common::SDString("1234567890"));
}

TEST(StringTest, ne_op) {
	i8 buf[] = "1234667890";
	Common::SDString s("1234567890"), s1("1234667890");
	EXPECT_NE(s, buf);
	EXPECT_NE(s, s1);
	EXPECT_TRUE(s != "1234667890");
	EXPECT_TRUE(s != Common::SDString("1234667890"));
}

TEST(StringTest, lt_op) {
	i8 buf[] = "1234667890";
	Common::SDString s("1234567890"), s1("1234667890");
	EXPECT_LT(s, buf);
	EXPECT_LT(s, s1);
	EXPECT_TRUE(s < "1234667890");
	EXPECT_TRUE(s < Common::SDString("1234667890"));

	EXPECT_LE(s, buf);
	EXPECT_LE(s, s1);
	EXPECT_TRUE(s <= "1234667890");
	EXPECT_TRUE(s <= Common::SDString("1234667890"));
}

TEST(StringTest, gt_op) {
	i8 buf[] = "123456789";
	Common::SDString s("1234567890"), s1("123456789");
	EXPECT_GT(s, buf);
	EXPECT_GT(s, s1);
	EXPECT_TRUE(s > "123456789");
	EXPECT_TRUE(s > Common::SDString("123456789"));

	EXPECT_GE(s, buf);
	EXPECT_GE(s, s1);
	EXPECT_TRUE(s >= "123456789");
	EXPECT_TRUE(s >= Common::SDString("123456789"));
}