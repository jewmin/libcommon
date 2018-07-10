#ifndef __UNIT_TEST_THREAD_TEST_H__
#define __UNIT_TEST_THREAD_TEST_H__

#include "gmock/gmock.h"
#include "thread.h"

class MockThread : public BaseThread
{
public:
    static int ThreadCalled;
    static int ThreadTerminated;

protected:
    virtual void Run()
    {
        ThreadCalled++;
    }

    virtual void OnTerminated()
    {
        ThreadTerminated++;
    }
};

class MockLoopThread : public BaseThread
{
public:
    static int ThreadCalled;
    static int ThreadTerminated;

protected:
    virtual void Run()
    {
        while (!IsTerminated())
        {
            ThreadCalled++;
            Sleep(1);
        }
    }

    virtual void OnTerminated()
    {
        ThreadTerminated++;
    }
};

class MockStopThread : public BaseThread
{
public:
    void Stop()
    {
        Terminate();
        BaseThread::Stop();
    }

protected:
    virtual void Run()
    {
        while (!IsTerminated())
        {
            Sleep(10);
        }
    }

    virtual void OnTerminated()
    {
        
    }
};

#endif