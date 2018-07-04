#include "thread.h"

BaseThread::BaseThread()
{
    _tid = 0;
    _terminated = false;
}

BaseThread::~BaseThread()
{
    Stop();
}

int BaseThread::Start()
{
    return uv_thread_create(&_tid, Run, this);
}

void BaseThread::Stop()
{
    Terminate();
    uv_thread_join(&_tid);
}