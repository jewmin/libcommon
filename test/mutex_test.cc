#include "gtest/gtest.h"
#include "mutex.h"

TEST(MutexTest, thread_mutex)
{
    Mutex mutex;
    mutex.Lock();
    mutex.Unlock();
}

TEST(MutexTest, thread_mutex_recursive)
{
    Mutex mutex(true);
    mutex.Lock();
    mutex.Lock();
    mutex.TryLock();

    mutex.Unlock();
    mutex.Unlock();
    mutex.Unlock();
}

TEST(MutexTest, owner)
{
    Mutex mutex;
    {
        Mutex::Owner lock(mutex);
    }
    mutex.Lock();
    mutex.Unlock();
}