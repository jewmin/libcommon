#include "gtest/gtest.h"
#include "exception.h"

TEST(Exception, try_catch)
{
    try
    {
        throw BaseException("Exception", "try_catch");
    }
    catch (const BaseException & ex)
    {
        EXPECT_STREQ(ex.Where().c_str(), "Exception");
        EXPECT_STREQ(ex.Message().c_str(), "try_catch");
    }
}