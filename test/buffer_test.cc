#include "gtest/gtest.h"
#include "buffer_test.h"
#include "exception.h"

TEST(AllocatorTest, allocator)
{
    Buffer::Allocator alloc(1024, 0);
    Buffer * buf = alloc.Allocate();
    buf->Release();
}

TEST(AllocatorTest, alloc_from_free)
{
    Buffer::Allocator alloc(1024, 0);
    Buffer * buf = alloc.Allocate();
    buf->Release();
    buf = alloc.Allocate();
    buf->Release();
}

TEST(AllocatorTest, release_error)
{
    try
    {
        Buffer::Allocator alloc(1024, 0);
        Buffer * buf = alloc.Allocate();
        buf->Release();
        buf->Release();
    }
    catch (const BaseException & ex)
    {
        EXPECT_STREQ(ex.Where().c_str(), "Buffer::Release()");
        EXPECT_STREQ(ex.Message().c_str(), "_ref is already zero");
    }
}

TEST(AllocatorTest, release_destroy)
{
    Buffer::Allocator alloc(1024, 1);
    Buffer * buf = alloc.Allocate();
    Buffer * buf2 = alloc.Allocate();
    buf->Release();
    buf2->Release();
}

TEST(AllocatorTest, flush)
{
    Buffer::Allocator alloc(1024, 1);
    Buffer * buf = alloc.Allocate();
    buf->Release();
    buf = alloc.Allocate();
}

TEST(BufferTest, add_data)
{
    Buffer::Allocator alloc(1024, 0);
    Buffer * buf = alloc.Allocate();
    buf->AddData('Z');
    EXPECT_EQ(buf->GetSize(), 1024);
    EXPECT_EQ(buf->GetUsed(), 1);

    char a[] = "abcdefghijklmnopqrstuvwxyz";
    buf->AddData(a, strlen(a));
    EXPECT_EQ(buf->GetSize(), 1024);
    EXPECT_EQ(buf->GetUsed(), 27);

    int b = 0x01020304;
    buf->AddData((const uint8_t *)&b, sizeof(b));
    EXPECT_EQ(buf->GetSize(), 1024);
    EXPECT_EQ(buf->GetUsed(), 31);
}

TEST(BufferTest, operation)
{
    Buffer::Allocator alloc(1024, 0);
    Buffer * buf = alloc.Allocate();

    uv_buf_t * b = buf->GetUVBuffer();
    EXPECT_EQ(b->len, 1024);

    buf->SetupRead();
    b = buf->GetUVBuffer();
    EXPECT_EQ(b->len, 1024);

    char a[] = "abcdefghijklmnopqrstuvwxyz";
    buf->AddData(a, strlen(a));
    buf->AddData(0);

    buf->SetupRead();
    b = buf->GetUVBuffer();
    EXPECT_EQ(b->len, 1024 - 27);

    buf->SetupWrite();
    b = buf->GetUVBuffer();
    EXPECT_EQ(b->len, 27);
}

TEST(BufferTest, split)
{
    Buffer::Allocator alloc(1024, 0);
    Buffer * buf = alloc.Allocate();

    char a[] = "abcdefghijklmnopqrstuvwxyz";
    buf->AddData(a, strlen(a));
    buf->AddData(0);

    Buffer * c = buf->SplitBuffer(10);
    c->AddData(0);
    EXPECT_STREQ((const char *)c->GetBuffer(), "abcdefghij");
}

//TEST(BufferTest, add_error)
//{
//    try
//    {
//        Buffer::Allocator alloc(10, 0);
//        Buffer * buf = alloc.Allocate();
//        buf->Use(10);
//        buf->AddData(0);
//    }
//    catch (const BaseException & ex)
//    {
//        EXPECT_STREQ(ex.Where().c_str(), "Buffer::AddData()");
//        EXPECT_STREQ(ex.Message().c_str(), "Not enough space in buffer");
//    }
//}