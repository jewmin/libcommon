#ifndef __LIB_COMMON_THREAD_H__
#define __LIB_COMMON_THREAD_H__

#include "uv.h"

class BaseThread
{
public:
    BaseThread();
    virtual ~BaseThread();
    int Start();
    virtual void Stop();

    inline void Terminate() { this->_terminated = true; }
    inline bool IsTerminated() { return this->_terminated; }

protected:
    //�̴߳�����������̳�
    virtual void Run() = 0;
    //�߳���ֹ���֪ͨ����
    virtual void OnTerminated();

private:
    static void Callback(void * arg);

    /*
     * No copies do not implement
     */
    BaseThread(const BaseThread & rhs);
    BaseThread & operator =(const BaseThread & rhs);

protected:
    uv_thread_t _tid;
    bool _terminated;
};

#endif