#include "thread_test.h"
#include "gtest/gtest.h"

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
    Sleep(10);
    thread.Stop();
    EXPECT_GT(MockLoopThread::ThreadCalled, 1);
    EXPECT_EQ(MockLoopThread::ThreadTerminated, 1);
}