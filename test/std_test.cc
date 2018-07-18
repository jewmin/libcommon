#include "gtest/gtest.h"
#include <deque>
#include <queue>
#include <vector>
#include "time.h"

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
    std::vector<int> vec;
    for (int i = 0; i < 10000; i++)
        vec.push_back(i);

    std::vector<int> dst_vec;
    for (int i = -1; i > -10000; i--)
        dst_vec.push_back(i);

    clock_t start = clock();
    dst_vec.insert(dst_vec.end(), vec.begin(), vec.end());
    clock_t end = clock();
    printf("std::vector insert run time: %ld, count: %d\n", end - start, dst_vec.size());
}