#include "thread.h"

BaseThread::BaseThread()
{
    _tid = 0;
    _terminated = false;
}

BaseThread::~BaseThread()
{

}

int BaseThread::Start()
{
    return uv_thread_create(&_tid, BaseThread::Callback, this);
}

void BaseThread::Stop()
{
    if (_tid != 0)
    {
        this->Terminate();
        uv_thread_join(&_tid);
    }
}

void BaseThread::OnTerminated()
{

}

void BaseThread::Callback(void * arg)
{
    BaseThread * thread = (BaseThread *)arg;
    thread->Run();
    thread->OnTerminated();
    thread->_tid = 0;
}