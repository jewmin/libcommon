#include "gtest/gtest.h"
#include "Buffer/CBuffer.h"
#include "Buffer/StraightBuffer.h"
#include "Buffer/BipBuffer.h"

class MockIOBuffer : public Common::CBuffer {
public:
	MockIOBuffer() : s(0) {}
	virtual ~MockIOBuffer() {}
	virtual i8 * WritableBlock(i32 want_size, i32 & writable_size) override {
		if (want_size > capacity_ - s) {
			writable_size = capacity_ - s;
		} else {
			writable_size = want_size;
		}
		return buffer_ + s;
	}
	virtual i8 * ReadableBlock(i32 & readable_size) override {
		readable_size = s;
		return buffer_;
	}
	virtual void IncWriterIndex(i32 size) override {
		s += size;
	}
	virtual void IncReaderIndex(i32 size) override {
		if (s > size) {
			s -= size;
			std::memmove(buffer_, buffer_ + size, s);
		} else {
			s = 0;
		}
	}
	virtual i32 ReadableBytes() const override {
		return s;
	}
	virtual i32 WritableBytes() const override {
		return capacity_ - s;
	}
	virtual bool IsReadable() const override {
		return s > 0;
	}
	virtual bool IsWritable() const override {
		return capacity_ - s > 0;
	}
	i8 * GetBuffer() const {
		return buffer_;
	}
	i32 GetBufferLength() const {
		return capacity_;
	}
	void Release() {
		DeAllocate();
	}

	i32 s;
};

TEST(IOBufferTest, io_ctor) {
	MockIOBuffer io;
	EXPECT_TRUE(io.GetBuffer() == nullptr);
	EXPECT_EQ(io.GetBufferLength(), 0);
}

TEST(IOBufferTest, io_ctor2) {
	MockIOBuffer io;
	io.Allocate(10);
	EXPECT_TRUE(io.GetBuffer() != nullptr);
	EXPECT_EQ(io.GetBufferLength(), 10);
	io.Release();
}

TEST(IOBufferTest, io_ctor3) {
	MockIOBuffer io;
	io.Allocate(10);
	io.Allocate(20);
	EXPECT_TRUE(io.GetBuffer() != nullptr);
	EXPECT_EQ(io.GetBufferLength(), 10);
}

class MockIOBufferTest : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		io_.Allocate(10);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		io_.Release();
	}

	MockIOBuffer io_;
};

TEST_F(MockIOBufferTest, io_write) {
	EXPECT_EQ(io_.IsWritable(), true);
	EXPECT_EQ(io_.WriteBytes("abcd", 4), 4);
	EXPECT_EQ(io_.IsWritable(), true);
	EXPECT_EQ(io_.WriteBytes("abcdefg", 7), 6);
	EXPECT_EQ(io_.IsWritable(), false);
	EXPECT_EQ(io_.WriteBytes("abc", 3), -1);
}

class MockReadIOBufferTest : public MockIOBufferTest {
public:
	MockReadIOBufferTest() {
		std::memset(buffer_, 0, sizeof(buffer_));
		std::memset(small_buffer_, 0, sizeof(small_buffer_));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		MockIOBufferTest::SetUp();
		io_.WriteBytes("abcdefghijklmnopqrstuvwxyz", static_cast<i32>(std::strlen("abcdefghijklmnopqrstuvwxyz")));
		std::memset(buffer_, 0, sizeof(buffer_));
		std::memset(small_buffer_, 0, sizeof(small_buffer_));
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		MockIOBufferTest::TearDown();
	}

	i8 buffer_[20];
	i8 small_buffer_[8];
};

TEST_F(MockReadIOBufferTest, io_read) {
	EXPECT_EQ(io_.IsReadable(), true);
	EXPECT_EQ(io_.ReadBytes(buffer_, sizeof(buffer_)), 10);
	EXPECT_STREQ(buffer_, "abcdefghij");
}

TEST_F(MockReadIOBufferTest, io_read_repeat) {
	EXPECT_EQ(io_.IsReadable(), true);
	EXPECT_EQ(io_.ReadBytes(small_buffer_, sizeof(small_buffer_)), 8);
	small_buffer_[7] = 0;
	EXPECT_STREQ(small_buffer_, "abcdefg");
	EXPECT_EQ(io_.IsReadable(), true);
	EXPECT_EQ(io_.ReadBytes(small_buffer_, sizeof(small_buffer_)), 2);
	small_buffer_[2] = 0;
	EXPECT_STREQ(small_buffer_, "ij");
	EXPECT_EQ(io_.IsReadable(), false);
	EXPECT_EQ(io_.ReadBytes(small_buffer_, sizeof(small_buffer_)), 0);
}

class MockStraightBufferTestSuite : public MockReadIOBufferTest {
public:
	MockStraightBufferTestSuite() : actually_size_(0), sb_(nullptr) {
		std::memset(read_buffer_, 0, sizeof(read_buffer_));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		MockReadIOBufferTest::SetUp();
		std::memset(read_buffer_, 0, sizeof(read_buffer_));
		actually_size_ = 0;
		sb_ = new Common::StraightBuffer();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete sb_;
		MockReadIOBufferTest::TearDown();
	}

	void WriteReadBuffer(i8 * data, i32 len) {
		if (data > 0) {
			std::memcpy(read_buffer_, data, len);
		}
		read_buffer_[len] = 0;
	}

	i8 read_buffer_[50];
	i32 actually_size_;
	Common::StraightBuffer * sb_;
};

TEST_F(MockStraightBufferTestSuite, ctor) {
	EXPECT_EQ(sb_->IsWritable(), false);
	EXPECT_TRUE(sb_->WritableBlock(10, actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_TRUE(sb_->ReadableBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	sb_->IncWriterIndex(10);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 0);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), false);

	sb_->IncWriterIndex(0);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 0);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), false);

	sb_->IncReaderIndex(-10);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 0);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), false);

	sb_->IncReaderIndex(0);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 0);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), false);

	sb_->IncReaderIndex(10);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 0);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), false);
}

TEST_F(MockStraightBufferTestSuite, alloc) {
	sb_->Allocate(20);

	EXPECT_TRUE(sb_->WritableBlock(10, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 10);
	EXPECT_TRUE(sb_->WritableBlock(20, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);
	EXPECT_TRUE(sb_->WritableBlock(30, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);

	EXPECT_TRUE(sb_->ReadableBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	sb_->IncWriterIndex(0);

	sb_->IncWriterIndex(10);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 20);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), true);

	sb_->IncWriterIndex(0);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 20);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), true);

	sb_->IncReaderIndex(-10);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 20);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), true);

	sb_->IncReaderIndex(0);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 20);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), true);

	sb_->IncReaderIndex(10);
	EXPECT_EQ(sb_->ReadableBytes(), 0);
	EXPECT_EQ(sb_->WritableBytes(), 20);
	EXPECT_EQ(sb_->IsReadable(), false);
	EXPECT_EQ(sb_->IsWritable(), true);
}

TEST_F(MockStraightBufferTestSuite, write) {
	char * block = nullptr;
	char * cblock = nullptr;
	const char * content = "123456";
	i32 content_len = static_cast<i32>(std::strlen(content));
	sb_->Allocate(20);

	block = sb_->WritableBlock(content_len, actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------

	block = sb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 14);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456");
	// -----------------------------------------
	// |1|2|3|4|5|6| | | | | | | | | | | | | | |
	// -----------------------------------------

	block = sb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 8);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6| | | | | | | | |
	// -----------------------------------------

	block = sb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 2);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456123456123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6|1|2|3|4|5|6| | |
	// -----------------------------------------

	block = sb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 2);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612345612345612");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6|1|2|3|4|5|6|1|2|
	// -----------------------------------------

	block = sb_->WritableBlock(content_len, actually_size_);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(actually_size_, 0);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612345612345612");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6|1|2|3|4|5|6|1|2|
	// -----------------------------------------

	sb_->IncReaderIndex(8);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 8);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "345612345612");
	// -----------------------------------------
	// | | | | | | | | |3|4|5|6|1|2|3|4|5|6|1|2|
	// -----------------------------------------

	sb_->IncReaderIndex(4);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 12);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612");
	// -----------------------------------------
	// | | | | | | | | | | | | |1|2|3|4|5|6|1|2|
	// -----------------------------------------
	
	block = sb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 6);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|1|2|3|4|5|6| | | | | | |
	// -----------------------------------------

	sb_->IncReaderIndex(50);
	cblock = sb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(sb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(sb_->WritableBytes(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------
}

class MockBipBufferTestSuite : public MockStraightBufferTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		MockStraightBufferTestSuite::SetUp();
		bb_ = new Common::BipBuffer();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete bb_;
		MockStraightBufferTestSuite::TearDown();
	}

	Common::BipBuffer * bb_;
};

TEST_F(MockBipBufferTestSuite, ctor) {
	EXPECT_TRUE(bb_->WritableBlock(10, actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	EXPECT_TRUE(bb_->ReadableBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	bb_->IncWriterIndex(10);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), false);

	bb_->IncWriterIndex(0);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), false);

	bb_->IncReaderIndex(-10);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), false);

	bb_->IncReaderIndex(0);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), false);

	bb_->IncReaderIndex(10);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), false);
}

TEST_F(MockBipBufferTestSuite, alloc) {
	bb_->Allocate(20);

	EXPECT_TRUE(bb_->WritableBlock(10, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 10);
	EXPECT_TRUE(bb_->WritableBlock(20, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);
	EXPECT_TRUE(bb_->WritableBlock(30, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);

	EXPECT_TRUE(bb_->ReadableBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	bb_->IncWriterIndex(0);

	bb_->IncWriterIndex(10);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 20);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), true);

	bb_->IncWriterIndex(0);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 20);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), true);

	bb_->IncReaderIndex(-10);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 20);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), true);

	bb_->IncReaderIndex(0);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 20);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), true);

	bb_->IncReaderIndex(10);
	EXPECT_EQ(bb_->ReadableBytes(), 0);
	EXPECT_EQ(bb_->WritableBytes(), 20);
	EXPECT_EQ(bb_->IsReadable(), false);
	EXPECT_EQ(bb_->IsWritable(), true);
}

TEST_F(MockBipBufferTestSuite, write) {
	char * block = nullptr;
	char * cblock = nullptr;
	const char * content = "1234567";
	i32 content_len = static_cast<i32>(std::strlen(content));
	bb_->Allocate(20);

	block = bb_->WritableBlock(content_len, actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 13);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "1234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | | | | | | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 6);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345671234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7| | | | | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345671234567123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7|1|2|3|4|5|6|
	// -----------------------------------------

	bb_->IncReaderIndex(10);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 10);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4567123456");
	// -----------------------------------------
	// | | | | | | | | | | |4|5|6|7|1|2|3|4|5|6|
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 3);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4567123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | |4|5|6|7|1|2|3|4|5|6|
	// -----------------------------------------

	bb_->IncReaderIndex(4);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 7);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | |1|2|3|4|5|6|
	// -----------------------------------------

	bb_->IncReaderIndex(40);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 13);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "1234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | | | | | | | |
	// -----------------------------------------

	block = bb_->WritableBlock(4, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 4);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 9);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345671234");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4| | | | | | | | | |
	// -----------------------------------------

	bb_->IncReaderIndex(10);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 10);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4");
	// -----------------------------------------
	// | | | | | | | | | | |4| | | | | | | | | |
	// -----------------------------------------

	block = bb_->WritableBlock(14, actually_size_);
	std::memcpy(block, "1234567890", actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 10);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0|4| | | | | | | | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(actually_size_, 0);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0|4| | | | | | | | | |
	// -----------------------------------------

	bb_->IncReaderIndex(10);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 10);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "1234567890");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0| | | | | | | | | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 3);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345678901234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7| | | |
	// -----------------------------------------

	bb_->IncReaderIndex(15);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 15);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | |6|7| | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 8);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | | |6|7| | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 1);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7| |6|7| | | |
	// -----------------------------------------

	block = bb_->WritableBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->IncWriterIndex(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 1);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7|1|6|7| | | |
	// -----------------------------------------

	bb_->IncReaderIndex(20);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 5);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456712345671");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7|1| | | | | |
	// -----------------------------------------

	bb_->IncReaderIndex(20);
	cblock = bb_->ReadableBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(bb_->ReadableBytes(), actually_size_);
	EXPECT_EQ(bb_->WritableBytes(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------
}