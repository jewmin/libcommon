#include "gtest/gtest.h"
#include "service.h"

TEST(ServiceTest, RunLoop)
{
    BaseService service;
    service.Start();
    Sleep(300000);
    service.Stop();
}