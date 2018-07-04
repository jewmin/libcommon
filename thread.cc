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
    Terminate();
    uv_thread_join(&_tid);
}

void BaseThread::Callback(void * arg)
{
    (BaseThread *)ptr = (BaseThread *)arg;
    ptr->Run();
}