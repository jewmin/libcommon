#include "thread_test.h"
#include "gtest/gtest.h"
#include "common.h"

int MockThread::ThreadCalled = 0;
int MockThread::ThreadTerminated = 0;
int MockLoopThread::ThreadCalled = 0;
int MockLoopThread::ThreadTerminated = 0;

TEST(ThreadTest, thread_create)
{
    MockThread thread;
    thread.Start();
    thread.Stop();
    EXPECT_EQ(MockThread::ThreadCalled, 1);
    EXPECT_EQ(MockThread::ThreadTerminated, 1);
}

TEST(ThreadTest, thread_loop)
{
    MockLoopThread thread;
    thread.Start();
    jc_sleep(10);
    thread.Stop();
    EXPECT_GT(MockLoopThread::ThreadCalled, 1);
    EXPECT_EQ(MockLoopThread::ThreadTerminated, 1);
}

TEST(ThreadTest, thread_stop)
{
    MockStopThread * pd = new MockStopThread();
    pd->Start();
    pd->Stop();

    BaseThread * pb = new MockStopThread();
    pb->Start();
    pb->Stop();

    delete pd;
    delete pb;
}

TEST(ThreadTest, judgment)
{
    MockSelfThread thread;
    thread.Start();
    jc_sleep(50);
    EXPECT_EQ(thread.IsCurrentThread(), false);
    EXPECT_EQ(thread.bIsCurrentThread, true);
    thread.Stop();
}