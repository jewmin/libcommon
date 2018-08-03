#include "gtest/gtest.h"
#include <deque>
#include <queue>
#include <vector>
#include "time.h"
#include "lock_queue.h"
#include "double_buffer.h"

TEST(StdTest, deque)
{
    std::deque<int> a;
    std::deque<int> b;
    a.push_back(10);
    a.push_back(20);
    a.push_back(30);
    b.assign(a.begin(), a.end());
    printf("a.count = %llu, b.count = %llu, a.max = %llu, b.max = %llu\n", a.size(), b.size(), a.max_size(), b.max_size());
    b.pop_front();
    printf("a.front = %d, b.front = %d\n", a.front(), b.front());
}

TEST(StdTest, queue)
{
    std::queue<int> a;
    a.push(10);
    a.push(20);
    a.push(30);
    a.front();
    a.pop();
    a.size();
    a.back();
    a.empty();
}

TEST(StdTest, vector)
{
    int count = 10000;
    std::vector<int> vec;
    vec.reserve(count);
    for (int i = 0; i < count; i++)
        vec.push_back(i);

    std::vector<int> dst_vec;
    dst_vec.reserve(count);
    for (int i = -1; i > -count; i--)
        dst_vec.push_back(i);

    clock_t start = clock();
    dst_vec.reserve(dst_vec.size() + vec.size());
    dst_vec.insert(dst_vec.end(), vec.begin(), vec.end());
    clock_t end = clock();
    printf("std::vector insert run time: %ld, count: %llu\n", end - start, dst_vec.size());
}

TEST(LockQueueTest, mutex)
{
    LockQueue<int> lq;
    lq.Push(100);
    lq.Push(200);
    EXPECT_EQ(lq.GetAppendSize(), 2);
    EXPECT_EQ(lq.size(), 0);
    lq.Flush();
    EXPECT_EQ(lq.GetAppendSize(), 0);
    EXPECT_EQ(lq.size(), 2);

    std::vector<int> vec;
    vec.push_back(300);
    vec.push_back(400);
    lq.Push(vec);
    EXPECT_EQ(lq.GetAppendSize(), 2);
    lq.Flush();
    EXPECT_EQ(lq[2], 300);
}

TEST(StdTest, set)
{
    std::set<int> s;
    s.insert(1);
    s.insert(2);
    s.insert(3);
    std::set<int>::iterator it = s.find(3);
    if (it != s.end())
        s.erase(it);

    it = s.find(4);
    if (it != s.end())
        s.erase(it);

    s.clear();
    it = s.find(2);
    if (it != s.end())
        s.erase(it);

    s.erase(4);
}

TEST(DoubleBufferTest, use)
{
    DoubleBuffer<int> db;
    db.Push(10);
    db.Push(20);
    db.Push(30);
    db.Push(40);
    EXPECT_EQ(db.Size(), 4);
    EXPECT_EQ(db.Pop(), 10);
    EXPECT_EQ(db.Pop(), 20);
    int i = 2;
    while (!db.Empty())
    {
        int data = db.Front();
        EXPECT_EQ(data, ++i * 10);
        db.PopFront();
    }
}