#include "gtest/gtest.h"
#include "object_pool.hpp"

TEST(ObjectPoolTest, use)
{
    ObjectPool<int> pool;
    int * p1 = pool.Allocate();
    int * p2 = pool.Allocate();
    pool.Release(p1);
    pool.ReleaseList(&p2, 1);
    pool.Clear();
}

TEST(ObjectPoolTest, use_struct)
{
    struct AA {
        int a;
        int b;
        int c;
    };

    ObjectPool<AA> pool;
    AA * p1 = pool.Allocate();
    AA * p2 = pool.Allocate();
    pool.Release(p1);
    pool.ReleaseList(&p2, 1);
}