#include "gtest/gtest.h"
#ifdef USE_VLD
#include "vld.h"
#endif

int main(int argc, char * * argv) {
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	return result;
}