#ifndef __LIBCOMMON_THREAD_H__
#define __LIBCOMMON_THREAD_H__

#include "uv.h"
#include "non_copy_able.hpp"

class BaseThread : public NonCopyAble
{
public:
    BaseThread();
    virtual ~BaseThread();

    // 启动线程成功则返回0
    int Start();
    // 必须显示调用停止线程
    virtual void Stop();
    // 终止线程运行
    inline void Terminate() { terminated_ = true; }
    // 线程已终止则返回true
    inline bool IsTerminated() { return terminated_; }

protected:
    // 线程处理函数，子类继承
    virtual void Run() = 0;
    // 线程终止后的通知函数
    virtual void OnTerminated();

private:
    static void Callback(void * arg);

protected:
    uv_thread_t thread_;
    bool terminated_;
};

#endif