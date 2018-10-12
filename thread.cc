#include "thread.h"

BaseThread::BaseThread() {
    thread_ = 0;
    terminated_ = false;
}

BaseThread::~BaseThread() {

}

int BaseThread::Start() {
    return uv_thread_create(&thread_, BaseThread::Callback, this);
}

void BaseThread::Stop() {
    if (thread_ != 0) {
        Terminate();
        uv_thread_join(&thread_);
    }
}

void BaseThread::OnTerminated() {

}

void BaseThread::Callback(void * arg) {
    BaseThread * thread = static_cast<BaseThread *>(arg);
    thread->Run();
    thread->OnTerminated();
    thread->thread_ = 0;
}