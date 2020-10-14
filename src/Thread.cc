/*
 * MIT License
 *
 * Copyright (c) 2019 jewmin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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