#include "gtest/gtest.h"
#include "object_pool.hpp"
#include "packet_pool.hpp"
#include <vector>

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

TEST(PacketPoolTest, gc)
{
    PacketPool pool;
    std::vector<Packet *> vec;
    for (int i = 0; i < 10; i++) {
        Packet * p = pool.Allocate();
        p->WriteString("hello world");
        vec.push_back(p);
    }
    for (int i = 0; i < 10; i++) {
        Packet * p = vec[i];
        pool.Release(p);
    }
    pool.GC();
}