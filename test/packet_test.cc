#include "gtest/gtest.h"
#include "packet.hpp"
#include "packet_reader.hpp"

typedef struct test_struct {
    int a;
    uint32_t b;
    short c;
    uint16_t d;
    char e;
    uint8_t f;
} test_struct;

TEST(PacketTest, use)
{
    const char * null_str = nullptr;
    const char * hello_str = "hello world!";
    char str[128] = "I'm a superman, nice to meet you.";
    char buf[128] = {0};
    uint8_t a = 0x12;
    char b = 0x34;
    short c = 22222;
    uint16_t d = 55555;
    int e = 999999999;
    uint32_t f = 3333333333;
    int64_t g = 0x4444444444444444;
    uint64_t h = 0xaaaaaaaaaaaaaaaa;
    Packet writer, writer2(reinterpret_cast<uint8_t *>(buf), 128);
    writer.SetAllocSize(1024);
    writer.WriteAtom<uint8_t>(a);
    writer.WriteAtom<char>(b);
    writer.WriteAtom<short>(c);
    writer.WriteAtom<uint16_t>(d);
    writer.WriteAtom<int>(e);
    writer.WriteAtom<uint32_t>(f);
    writer.WriteAtom<int64_t>(g);
    writer.WriteAtom<uint64_t>(h);
    test_struct ts = {1, 2, 3, 4, 5, 6};
    writer.WriteBinary(reinterpret_cast<const uint8_t *>(&ts), sizeof(ts));
    writer.WriteString(str);
    writer.WriteString(null_str);

    size_t size = sizeof(uint8_t) + sizeof(char);
    size += sizeof(short) + sizeof(uint16_t);
    size += sizeof(int) + sizeof(uint32_t);
    size += sizeof(int64_t) + sizeof(uint64_t);
    size += sizeof(ts);
    size += strlen(str) + sizeof(uint16_t) + 1;
    size += sizeof(uint16_t) + 1;

    writer2 << a << b << c << d << e << f << g;
    writer2 << ts;
    writer2 << str;
    writer2 << hello_str;

    writer.SetPosition(size + 10);
    writer.SetPosition(1500);
    writer.SetPosition(0);

    EXPECT_EQ(0, writer.AdjustOffset(-1));
    EXPECT_EQ(10, writer.AdjustOffset(10));
    EXPECT_EQ(3000, writer.AdjustOffset(2990));

    writer.SetLength(4025);
    writer.SetLength(size);

    EXPECT_EQ(5049, writer.Reserve(5050));
    EXPECT_EQ(6074, writer.Reserve(100));

    PacketReader reader, reader2(writer2.GetMemoryPtr(), writer2.GetLength());
    uint8_t a1;
    char b1;
    short c1;
    uint16_t d1;
    reader2 >> a1 >> b1 >> c1 >> d1;
    EXPECT_EQ(a1, 0x12);
    EXPECT_EQ(b1, 0x34);
    EXPECT_EQ(c1, 22222);
    EXPECT_EQ(d1, 55555);

    EXPECT_EQ(reader2.GetSize(), writer2.GetLength());
    EXPECT_EQ(reader2.GetLength(), writer2.GetLength());
    EXPECT_EQ(reader2.GetReadableLength(), reader2.GetLength() - 6);
    EXPECT_EQ(reader2.GetPosition(), 6);
    EXPECT_EQ(reader2.GetMemoryPtr(), writer2.GetMemoryPtr());
    EXPECT_EQ(reader2.GetOffsetPtr(), reader2.GetPositionPtr(6));

    int e1;
    uint32_t f1;
    int64_t g1;
    test_struct ts1;
    char hello_str1[128] = { 0 };
    memset(&ts1, 0, sizeof(ts1));
    const char * str1;
    reader2 >> e1 >> f1 >> g1 >> ts1 >> str1;
    reader2.ReadString(hello_str1, 128);
    EXPECT_EQ(e1, 999999999);
    EXPECT_EQ(f1, 3333333333);
    EXPECT_EQ(g1, 0x4444444444444444);
    EXPECT_EQ(ts1.a, 1);
    EXPECT_EQ(ts1.b, 2);
    EXPECT_EQ(ts1.c, 3);
    EXPECT_EQ(ts1.d, 4);
    EXPECT_EQ(ts1.e, 5);
    EXPECT_EQ(ts1.f, 6);
    EXPECT_STREQ(str1, "I'm a superman, nice to meet you.");
    EXPECT_STREQ(hello_str1, "hello world!");

    int error1;
    uint64_t error2;
    reader2 >> error1 >> error2;
    EXPECT_EQ(0, error1);
    EXPECT_EQ(0, error2);

    EXPECT_EQ(reader2.SetPosition(1024), writer2.GetLength());
    EXPECT_EQ(reader2.SetPosition(0), 0);

    EXPECT_EQ(0, reader2.AdjustOffset(-1));
    EXPECT_EQ(writer2.GetLength(), reader2.AdjustOffset(1024));
    reader2.SetPosition(0);
    EXPECT_EQ(50, reader2.AdjustOffset(50));

    reader2.SetPosition(1024);
    const char * null_str1;
    reader2 >> null_str1;
    ASSERT_TRUE(null_str1 == nullptr);

    size_t size1 = sizeof(uint8_t) + sizeof(char);
    size1 += sizeof(short) + sizeof(uint16_t);
    size1 += sizeof(int) + sizeof(uint32_t);
    size1 += sizeof(int64_t) + sizeof(test_struct);
    reader2.SetPosition(size1);
    size_t str_len = strlen("I'm a superman, nice to meet you.");
    char str2[128];
    reader2.ReadString(str2, str_len - 5);
    EXPECT_STREQ(str2, "I'm a superman, nice to mee");

    PacketReader reader3(writer2.GetMemoryPtr(), writer2.GetLength() - 8);
    reader3.SetPosition(size1);
    reader3 >> null_str1;
    reader3.ReadString(str2, 128);
    EXPECT_STREQ(str2, "hello");
}