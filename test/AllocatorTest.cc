#include "gtest/gtest.h"
#include "Allocator.h"

TEST(AllocatorTest, replace) {
	jc_malloc_func test_malloc[2] = { nullptr, std::malloc };
	jc_realloc_func test_realloc[2] = { nullptr, std::realloc };
	jc_calloc_func test_calloc[2] = { nullptr, std::calloc };
	jc_free_func test_free[2] = { nullptr, std::free };
	for (i32 i = 0; i < 16; ++i) {
		EXPECT_EQ(i == 15, jc_replace_allocator(test_malloc[i & 1], test_realloc[(i >> 1) & 1], test_calloc[(i >> 2) & 1], test_free[(i >> 3) & 1]));
	}
}

TEST(AllocatorTest, alloc) {
	i32 * p = static_cast<i32 *>(jc_malloc(sizeof(i32)));
	EXPECT_TRUE(p != nullptr);
	*p = 999;
	EXPECT_EQ(*p, 999);

	p = static_cast<i32 *>(jc_realloc(p, sizeof(i32) << 1));
	EXPECT_TRUE(p != nullptr);
	*(p + 1) = 88;
	EXPECT_EQ(*p, 999);
	EXPECT_EQ(*(p + 1), 88);

	jc_free(p);

	p = static_cast<i32 *>(jc_calloc(3, sizeof(i32)));
	EXPECT_TRUE(p != nullptr);
	EXPECT_EQ(*p, 0);
	EXPECT_EQ(*(p + 1), 0);
	EXPECT_EQ(*(p + 2), 0);

	jc_free(p);
}