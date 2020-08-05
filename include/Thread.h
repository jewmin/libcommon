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

#ifndef Common_Thread_INCLUDED
#define Common_Thread_INCLUDED

#include "CObject.h"

namespace Common {

class COMMON_EXTERN CThread : public CObject {
public:
	virtual ~CThread();

	void Start();
	void Wait();
	void Terminate();
	bool Terminated() const;
	std::thread::id GetThreadId() const;

	static void Sleep(i32 milliseconds);
	static std::thread::id CurrentThreadId();

protected:
	CThread();
	virtual void OnRountine() = 0;

private:
	static void ThreadRoutine(CThread * thread);

private:
	std::thread * thread_;
	bool terminated_;
};

inline void CThread::Terminate() {
	terminated_ = true;
}

inline bool CThread::Terminated() const {
	return terminated_;
}

inline std::thread::id CThread::GetThreadId() const {
	if (thread_) {
		return thread_->get_id();
	} else {
		return std::thread::id();
	}
}

}

#endif