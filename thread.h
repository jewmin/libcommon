#ifndef __LIBCOMMON_THREAD_H__
#define __LIBCOMMON_THREAD_H__

#include "uv.h"
#include "non_copy_able.hpp"

class BaseThread : public NonCopyAble
{
public:
    BaseThread();
    virtual ~BaseThread();

    // �����̳߳ɹ��򷵻�0
    int Start();
    // ������ʾ����ֹͣ�߳�
    virtual void Stop();
    // ��ֹ�߳�����
    inline void Terminate() { terminated_ = true; }
    // �߳�����ֹ�򷵻�true
    inline bool IsTerminated() { return terminated_; }
    // �ǵ�ǰ�߳��򷵻�true
    inline bool IsCurrentThread()
    {
        uv_thread_t current_thread = uv_thread_self();
        return !!uv_thread_equal(&thread_, &current_thread);
    }

protected:
    // �̴߳�����������̳�
    virtual void Run() = 0;
    // �߳���ֹ���֪ͨ����
    virtual void OnTerminated();

private:
    static void Callback(void * arg);

protected:
    uv_thread_t thread_;
    bool terminated_;
};

#endif