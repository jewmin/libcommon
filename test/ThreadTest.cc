#include "gtest/gtest.h"
#include "Thread.h"

class MockThread : public Logger::Thread {
public:
	MockThread(i32 milli_seconds) : milli_seconds_(milli_seconds) {}

protected:
	virtual void OnRountine() override {
		while (!Terminated()) {
			Logger::Thread::Sleep(milli_seconds_);
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
	EXPECT_STREQ(s.str().c_str(), "0");
}

TEST(ThreadTest, multi) {
	MockThread * t[10];
	for (i32 i = 0; i < 10; ++i) {
		t[i] = new MockThread((i + 1) * 5);
		t[i]->Start();
	}
	Logger::Thread::Sleep(200);
	for (i32 i = 0; i < 5; ++i) {
		t[i]->Terminate();
	}
	Logger::Thread::Sleep(300);
	for (i32 i = 0; i < 10; ++i) {
		delete t[i];
	}
}

TEST(ThreadTest, current_id) {
	std::cout << "id: " << Logger::Thread::CurrentThreadId() << std::endl;
}