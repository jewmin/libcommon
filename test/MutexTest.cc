#include "gtest/gtest.h"
#include "Mutex.h"

void MutexTest_thread_task(Common::CMutex * mutex, i32 * count) {
	if (mutex) { mutex->Lock(); }
	i32 tmp = *count;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	*count = tmp + 1;
	if (mutex) { mutex->Unlock(); }
}

void MutexTest_thread_task_scope(Common::CMutex * mutex, i32 * count, i32 * mutex_count) {
	i32 tmp = *count;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	*count = tmp + 1;
	{
		Common::CMutex::ScopedLock lock(*mutex);
		i32 mutex_tmp = *mutex_count;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		*mutex_count = mutex_tmp + 1;
	}
}

void MutexTest_thread_task_try(Common::CMutex * mutex, i32 * count) {
	if (mutex && mutex->TryLock()) {
		i32 tmp = *count;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		*count = tmp + 1;
		mutex->Unlock();
	} else {
		std::printf("trylock false\n");
	}
}

TEST(MutexTest, mutex) {
	i32 count = 0;
	Common::CMutex mutex;
	std::thread threads[5];
	for (i32 i = 0; i < 5; i++) {
		threads[i] = std::thread(MutexTest_thread_task, &mutex, &count);
	}
	for (auto & t: threads) {
		t.join();
	}
	EXPECT_EQ(count, 5);
	std::printf("multithread mutex count=%d\n", count);
}

TEST(MutexTest, no_mutex) {
	i32 count = 0;
	std::thread threads[5];
	for (i32 i = 0; i < 5; i++) {
		threads[i] = std::thread(MutexTest_thread_task, nullptr, &count);
	}
	for (auto & t: threads) {
		t.join();
	}
	EXPECT_NE(count, 5);
	std::printf("multithread no mutex count=%d\n", count);
}

TEST(MutexTest, scope) {
	i32 count = 0, mutex_count = 0;
	Common::CMutex mutex;
	std::thread threads[5];
	for (i32 i = 0; i < 5; i++) {
		threads[i] = std::thread(MutexTest_thread_task_scope, &mutex, &count, &mutex_count);
	}
	for (auto & t: threads) {
		t.join();
	}
	EXPECT_NE(count, 5);
	EXPECT_EQ(mutex_count, 5);
	std::printf("multithread scopedlock count=%d mutex_count=%d\n", count, mutex_count);
}

TEST(MutexTest, mutex_try) {
	i32 count = 0;
	Common::CMutex mutex;
	std::thread threads[5];
	for (i32 i = 0; i < 5; i++) {
		threads[i] = std::thread(MutexTest_thread_task_try, &mutex, &count);
	}
	for (auto & t: threads) {
		t.join();
	}
	EXPECT_LE(count, 5);
	std::printf("multithread mutex try count=%d\n", count);
}