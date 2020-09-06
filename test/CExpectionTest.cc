#include "gtest/gtest.h"
#include "CExpection.h"

TEST(CExpectionTest, default_ctor) {
	try {
		throw Common::CExpection();
	} catch (Common::CExpection & e) {
		EXPECT_STREQ(e.What(), "");
		std::printf("CExpection: %s\n", e.What());
	}
}

TEST(CExpectionTest, message_ctor) {
	try {
		throw Common::CExpection("test expection");
	} catch (Common::CExpection & e) {
		EXPECT_STREQ(e.What(), "test expection");
		std::printf("CExpection: %s\n", e.What());
	}
}

TEST(CExpectionTest, copy_ctor) {
	Common::CExpection exp("copy expection");
	Common::CExpection copy_exp(exp);
	EXPECT_STREQ(copy_exp.What(), "copy expection");
	EXPECT_STREQ(copy_exp.What(), exp.What());
}

TEST(CExpectionTest, move_ctor) {
	Common::CExpection exp("move expection");
	Common::CExpection move_exp(std::move(exp));
	EXPECT_STREQ(move_exp.What(), "move expection");
}

TEST(CExpectionTest, assign_op) {
	Common::CExpection exp("assign expection");
	Common::CExpection assign_exp;
	assign_exp = assign_exp;
	assign_exp = exp;
	EXPECT_STREQ(assign_exp.What(), "assign expection");
	EXPECT_STREQ(assign_exp.What(), exp.What());
}

TEST(CExpectionTest, move_op) {
	Common::CExpection exp("move assign expection");
	Common::CExpection move_assign_exp;
	move_assign_exp = std::move(move_assign_exp);
	move_assign_exp = std::move(exp);
	EXPECT_STREQ(move_assign_exp.What(), "move assign expection");
}