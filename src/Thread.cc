#include "Thread.h"
#include "Allocator.h"

namespace Common {

CThread::CThread() : thread_(nullptr), terminated_(false) {
}

CThread::~CThread() {
	Wait();
}

void CThread::Start() {
	if (!thread_) {
		// thread_ = new std::thread(ThreadRoutine, this);
		thread_ = static_cast<std::thread *>(jc_malloc(sizeof(*thread_)));
		new(thread_)std::thread(ThreadRoutine, this);
	}
}

void CThread::Wait() {
	if (thread_) {
		Terminate();
		thread_->join();
		// delete thread_;
		thread_->~thread();
		jc_free(thread_);
		thread_ = nullptr;
		terminated_ = false;
	}
}

void CThread::ThreadRoutine(CThread * thread) {
	thread->OnRountine();
}

void CThread::Sleep(i32 milliseconds) {
	std::this_thread::sleep_for(std::chrono::microseconds(milliseconds));
}

std::thread::id CThread::CurrentThreadId() {
	return std::this_thread::get_id();
}

}