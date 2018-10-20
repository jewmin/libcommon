#include "gtest/gtest.h"
#include "std_test.h"
#include <vector>
#include <functional>

TEST(LockQueueTest, mutex)
{
    LockQueue<int> lq;
    lq.Push(100);
    lq.Push(200);
    EXPECT_EQ(lq.AppendCount(), 2);
    EXPECT_EQ(lq.Count(), 0);
    lq.Flush();
    EXPECT_EQ(lq.AppendCount(), 0);
    EXPECT_EQ(lq.Count(), 2);

    BaseVector<int> vec;
    BaseVector<int, 20> vec1;
    vec.Add(300);
    vec.Add(400);
    vec1.Add(1);
    vec1.Add(2);
    lq.PushList(vec);
    lq.PushArray(static_cast<int *>(vec1), vec1.Count());
    EXPECT_EQ(lq.AppendCount(), 4);
    lq.Flush();
    EXPECT_EQ(lq[2], 300);
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

TEST(BaseVectorTest, use)
{
    BaseVector<int> a;
    EXPECT_EQ(a.Count(), 0);
    EXPECT_EQ(a.Capacity(), 0);
    ASSERT_TRUE(static_cast<int *>(a) == nullptr);

    int i;
    for (i = 0; i < 11; ++i)
        a.Add(i);

    EXPECT_EQ(a.Count(), 11);
    EXPECT_EQ(a.Capacity(), 20);
    ASSERT_TRUE(static_cast<int *>(a) != nullptr);
    
    int * p = static_cast<int *>(a);
    for (i = 0; i < 11; ++i)
        EXPECT_EQ(*(p + i), a[i]);

    BaseVector<int, 1024> b;
    b.AddVector(a);
    EXPECT_EQ(b.Count(), 11);
    EXPECT_EQ(b.Capacity(), 11);

    b.Trunc(5);
    EXPECT_EQ(b.Count(), 5);
    EXPECT_EQ(b.Capacity(), 11);

    b.Clear();
    EXPECT_EQ(b.Count(), 0);
    EXPECT_EQ(b.Capacity(), 11);

    b.Trunc(10);
    EXPECT_EQ(b.Count(), 10);
    EXPECT_EQ(b.Capacity(), 11);

    EXPECT_EQ(a[5], 5);
    a.Insert(5, 100);
    EXPECT_EQ(a[5], 100);
    EXPECT_EQ(a[6], 5);

    a.Remove(0);
    EXPECT_EQ(a[5], 5);
    a.Remove(5, 6);
    EXPECT_EQ(a.Count(), 5);

    BaseVector<int, 1024> c;
    c.Empty();
    c.Insert(0, 10);
    EXPECT_EQ(c.Capacity(), 1024);
    EXPECT_EQ(c.Get(0), 10);
    c.Set(0, 100);
    EXPECT_EQ(c[0], 100);

    EXPECT_EQ(a.Find(10), -1);
    EXPECT_EQ(a.Find(2), 1);
}

TEST(BaseListTest, use)
{
    TNode * node;
    TNodeList<TNode> a;
    a.PushNode(new TNode(1));
    a.PushNode(new TNode(2));
    node = a.PopNode();
    EXPECT_EQ(node->data_, 2);
    delete node;

    a.PushNode(new TNode(3));
    a.PushNode(new TNode(4));
    a.PushNode(new TNode(5));
    a.PushNode(new TNode(6));
    EXPECT_EQ(a.Count(), 5);
    EXPECT_EQ(a.IsEmpty(), false);
    node = a.Head();
    EXPECT_EQ(node->data_, 6);
    EXPECT_EQ(TNodeList<TNode>::Next(node)->data_, 5);

    a.PushBackNode(new TNode(100));
    a.PushBackNode(new TNode(101));
    a.PushBackNode(new TNode(102));
    EXPECT_EQ(a.Count(), 8);
    node = a.Tail();
    EXPECT_EQ(node->data_, 102);
    EXPECT_EQ(TNodeList<TNode>::Prev(node)->data_, 101);
    node = a.PopBackNode();
    EXPECT_EQ(node->data_, 102);
    EXPECT_EQ(a.Count(), 7);

    while (node = a.Head())
    {
        node->RemoveFromList();
        delete node;
    }
    EXPECT_EQ(a.IsEmpty(), true);
}

TEST(StdVectorTest, constructor)
{
    const char * hello_str = "hello world!";
    std::vector<char> v1(hello_str, hello_str + 13);
    for (auto & it : v1)
    {
        printf("%c", it);
    }
    printf("\n");
}

void func1()
{
    printf("func1\n");
}

void func2(int a)
{
    printf("func2 %d\n", a);
}

TEST(FunctionTest, use)
{
    /*LockQueue<std::function<void()>> func_queue;
    func_queue.Push(std::move(std::bind(func1)));
    func_queue.Push(std::bind(func2, 10));
    func_queue.Flush();
    for (int i = 0; i < func_queue.Count(); ++i)
    {
        func_queue[i]();
    }*/
	std::vector<std::function<void()>> func_queue;
	func_queue.push_back(std::bind(func1));
	func_queue.push_back(std::bind(func2, 10));
	for (auto & func : func_queue)
	{
		func();
	}
}