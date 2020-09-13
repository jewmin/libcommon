#include "gtest/gtest.h"
#include "Thread.h"

class MockThread : public Common::CThread {
public:
	MockThread(i32 milli_seconds) : milli_seconds_(milli_seconds) {}

protected:
	virtual void OnRountine() override {
		while (!Terminated()) {
			Common::CThread::Sleep(milli_seconds_);
			std::cout << "OnRountine: " << GetThreadId() << std::endl;
		}
	}

private:
	i32 milli_seconds_;
};

TEST(ThreadTest, terminate) {
	MockThread t(10);
	EXPECT_EQ(t.Terminated(), false);
	t.Terminate();
	EXPECT_EQ(t.Terminated(), true);
}

TEST(ThreadTest, id) {
	MockThread t(10);
	std::stringstream s;
	s << t.GetThreadId();
#ifdef _WIN32
	EXPECT_STREQ(s.str().c_str(), "0");
#else
	EXPECT_STREQ(s.str().c_str(), "thread::id of a non-executing thread");
#endif
}

TEST(ThreadTest, multi) {
	MockThread * t[10];
	for (i32 i = 0; i < 10; i++) {
		t[i] = new MockThread((i + 1) * 5);
		t[i]->Start();
	}
	Common::CThread::Sleep(200);
	for (i32 i = 0; i < 5; i++) {
		t[i]->Terminate();
	}
	Common::CThread::Sleep(300);
	for (i32 i = 0; i < 10; i++) {
		delete t[i];
	}
}

TEST(ThreadTest, current_id) {
	std::cout << "id: " << Common::CThread::CurrentThreadId() << std::endl;
}