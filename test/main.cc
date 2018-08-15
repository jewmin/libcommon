#include "uv.h"
#include "common.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

int main(int argc, char * * argv)
{
    uv_replace_allocator(jc_malloc, jc_realloc, jc_calloc, jc_free);
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}