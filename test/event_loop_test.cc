#include "gtest/gtest.h"
#include "event_loop.h"
#include "event_loop_thread.h"

void LoopExit(EventLoop * loop) {
    loop->Quit();
}

void LoopRun() {
    printf("LoopRun\n");
}

void LoopFunc(EventLoop * loop, uint32_t t1) {
    printf("LoopFunc\n");
    loop->Cancel(t1);
    loop->RunEvery(200, std::bind(&LoopRun));
}

void LoopFunc1(EventLoop * loop) {
    loop->RunAfter(500, std::bind(&LoopExit, loop));
}

TEST(EventLoopTest, t1)
{
    EventLoop loop;
    uint32_t t1 = loop.RunEvery(100, std::bind(&LoopRun));
    EXPECT_EQ(1, t1);
    loop.RunInLoop(std::bind(&LoopFunc, &loop, t1));
    loop.QueueInLoop(std::bind(&LoopFunc1, &loop));
    loop.Loop();
    if (loop.log()) {
        loop.log()->LogInfo("CloseLoop");
    }
}

TEST(EventLoopTest, t2)
{
    EventLoopThread thread;
    EventLoop * loop = thread.StartLoop();
    loop->RunInLoop(std::bind(&LoopRun));
}