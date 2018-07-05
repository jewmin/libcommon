#ifndef __LIB_COMMON_THREAD_H__
#define __LIB_COMMON_THREAD_H__

#include "uv.h"

class BaseThread
{
public:
    BaseThread();
    virtual ~BaseThread();
    int Start();
    void Stop();

    inline void Terminate() { _terminated = true; }
    inline bool IsTerminated() { return _terminated; }

protected:
    //线程处理函数，子类继承
    virtual void Run() = 0;
    //线程终止后的通知函数
    virtual void OnTerminated();

private:
    static void Callback(void * arg);

protected:
    uv_thread_t _tid;
    bool _terminated;
};

#endif