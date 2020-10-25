/*
 * MIT License
 *
 * Copyright (c) 2019 jewmin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef Common_Buffer_CBuffer_INCLUDED
#define Common_Buffer_CBuffer_INCLUDED

#include "Common.h"
#include "CObject.h"
#include "Allocator.h"

namespace Common {

// +-------------------+------------------+------------------+
// | discardable bytes |  readable bytes  |  writable bytes  |
// |                   |     (CONTENT)    |                  |
// +-------------------+------------------+------------------+
// |                   |                  |                  |
// 0      <=      readerIndex   <=   writerIndex    <=    capacity

class COMMON_EXTERN CBuffer : public CObject {
public:
	CBuffer() : buffer_(nullptr), capacity_(0) {}
	virtual ~CBuffer() { DeAllocate(); }

	void Allocate(i32 size);
	void DeAllocate();
	i32 WriteBytes(const i8 * data, i32 data_len);
	i32 ReadBytes(i8 * data, i32 data_len);

	virtual i8 * WritableBlock(i32 want_size, i32 & writable_size) = 0;
	virtual i8 * ReadableBlock(i32 & readable_size) = 0;
	virtual void IncWriterIndex(i32 size) = 0;
	virtual void IncReaderIndex(i32 size) = 0;
	virtual i32 ReadableBytes() const = 0;
	virtual i32 WritableBytes() const = 0;
	virtual bool IsReadable() const = 0;
	virtual bool IsWritable() const = 0;

private:
	CBuffer(CBuffer &&) = delete;
	CBuffer(const CBuffer &) = delete;
	CBuffer & operator=(CBuffer &&) = delete;
	CBuffer & operator=(const CBuffer &) = delete;

protected:
	i8 * buffer_;
	i32 capacity_;
};

inline void CBuffer::Allocate(i32 size) {
	if (!buffer_) {
		buffer_ = static_cast<i8 *>(jc_malloc(size));
		capacity_ = size;
	}
}

inline void CBuffer::DeAllocate() {
	if (buffer_) {
		jc_free(buffer_);
		buffer_ = nullptr;
		capacity_ = 0;
	}
}

inline i32 CBuffer::WriteBytes(const i8 * data, i32 data_len) {
	i32 writable_size = 0;
	i8 * block = WritableBlock(data_len, writable_size);
	if (block && writable_size > 0) {
		std::memcpy(block, data, writable_size);
		IncWriterIndex(writable_size);
		return writable_size;
	}
	return -1;
}

inline i32 CBuffer::ReadBytes(i8 * data, i32 data_len) {
	i32 readable_size = 0;
	i8 * block = ReadableBlock(readable_size);
	if (block && readable_size > 0) {
		if (readable_size < data_len) {
			data_len = readable_size;
		}
		std::memcpy(data, block, data_len);
		IncReaderIndex(data_len);
		return data_len;
	}
	return 0;
}

}

#endif