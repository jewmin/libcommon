#include "gtest/gtest.h"
#include "service_test.h"

TEST(ServiceTest, run_loop)
{
    BaseService service;
    service.Start();
    Sleep(10);
    service.Stop();
}

TEST(ServiceTest, msg)
{
    MockService service;
    service.Start();
	Sleep(10);
    for (int i = 0; i < 10; i++)
        service.PostMsg(i, 0, 0, 0, 0, 0);
    Sleep(10);
    service.Stop();

    EXPECT_EQ(service.recv_count, 10);
}