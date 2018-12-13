#include "gtest/gtest.h"
#include "std_test.h"
#include "uv.h"
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

//TEST(DoubleBufferTest, use)
//{
//    DoubleBuffer<int> db;
//    db.Push(10);
//    db.Push(20);
//    db.Push(30);
//    db.Push(40);
//    EXPECT_EQ(db.Size(), 4);
//    EXPECT_EQ(db.Pop(), 10);
//    EXPECT_EQ(db.Pop(), 20);
//    int i = 2;
//    while (!db.Empty())
//    {
//        int data = db.Front();
//        EXPECT_EQ(data, ++i * 10);
//        db.PopFront();
//    }
//}

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
    a.PushLeft(new TNode(1));
    a.PushLeft(new TNode(2));
    node = a.PopLeft();
    EXPECT_EQ(node->data_, 2);
    delete node;

    a.PushLeft(new TNode(3));
    a.PushLeft(new TNode(4));
    a.PushLeft(new TNode(5));
    a.PushLeft(new TNode(6));
    EXPECT_EQ(a.Count(), 5);
    EXPECT_EQ(a.IsEmpty(), false);
    node = a.Left();
    EXPECT_EQ(node->data_, 6);
    EXPECT_EQ(TNodeList<TNode>::Right(node)->data_, 5);

    a.PushRight(new TNode(100));
    a.PushRight(new TNode(101));
    a.PushRight(new TNode(102));
    EXPECT_EQ(a.Count(), 8);
    node = a.Right();
    EXPECT_EQ(node->data_, 102);
    EXPECT_EQ(TNodeList<TNode>::Left(node)->data_, 101);
    node = a.PopRight();
    EXPECT_EQ(node->data_, 102);
    EXPECT_EQ(a.Count(), 7);
    delete node;

    while (node = a.Left())
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

int * func1(int arg)
{
    return new int(arg);
}

TEST(FuncTest, use)
{
    std::function<int*(int)> func;
    func = std::bind(&func1, std::placeholders::_1);
    int * p = func(10);
    printf("%d\n", *p);
    delete p;

    TestClass tc;//implicit return type no suitable user-defined conversion from
    //tc.SetCallback(std::bind(&TestClass::test, &tc, std::placeholders::_1));
    tc.SetCallback2(std::bind(&TestClass::test2, &tc, std::placeholders::_1));
    p = tc.run();
    if (p) {
        printf("%d\n", *p);
        delete p;
    }
    
    tc.run2(&p);
    if (p) {
        printf("%d\n", *p);
        delete p;
    }
}

void func2(int arg1, int arg2) {
    printf("arg1=%d,arg2=%d\n", arg1, arg2);
}

TEST(FuncTest, use1)
{
    std::function<void(int)> func;
    func = std::bind(&func2, 10, std::placeholders::_1);
    func(20);
}

TEST(StdVectorTest, t1)
{
    const char * str = "hello world!";
    std::vector<char> storage1(str, str + strlen(str));
    std::vector<char> storage2;
    storage2.assign(str, str + strlen(str));
    for (int i = 0; i < storage1.size() && i < storage2.size(); i++)
    {
        EXPECT_EQ(storage1[i], storage2[i]);
    }
    EXPECT_EQ(storage1.size(), storage2.size());
}

TEST(BufTest, t1)
{
    const char * str = "hello world";
    uv_buf_t buf = uv_buf_init(const_cast<char *>(str), static_cast<unsigned int>(strlen(str)));
    EXPECT_STREQ(buf.base, str);
    EXPECT_EQ(buf.len, 11);
}

TEST(stringTest, use1)
{
    char msg[10] = { 0 };
    int ret = snprintf(msg, 10, "01234567890");
#ifdef _MSC_VER
    EXPECT_EQ(ret, -1);
#else
    EXPECT_EQ(ret, 11);
#endif
    ret = snprintf(msg, 10, "0123456789");
    EXPECT_EQ(ret, 10);
    ret = snprintf(msg, 10, "123456789");
    EXPECT_EQ(ret, 9);
}