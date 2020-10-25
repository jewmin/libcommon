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

#include "Buffer/StraightBuffer.h"

namespace Common {

StraightBuffer::StraightBuffer() : reader_index_(0), writer_index_(0), size_reserve_(0) {
}

StraightBuffer::~StraightBuffer() {
}

i8 * StraightBuffer::WritableBlock(i32 want_size, i32 & writable_size) {
	if (!IsWritable()) {
		writable_size = 0;
		return nullptr;
	}

	if (capacity_ - writer_index_ >= want_size) {
		size_reserve_ = writable_size = want_size;
		return buffer_ + writer_index_;
	}

	if (reader_index_ > 0) {
		std::memmove(buffer_, buffer_ + reader_index_, ReadableBytes());
		writer_index_ -= reader_index_;
		reader_index_ = 0;
	}

	if (capacity_ - writer_index_ < want_size) {
		want_size = capacity_ - writer_index_;
	}
	size_reserve_ = writable_size = want_size;
	return buffer_ + writer_index_;
}

i8 * StraightBuffer::ReadableBlock(i32 & readable_size) {
	if (!IsReadable()) {
		readable_size = 0;
		return nullptr;
	}

	readable_size = ReadableBytes();
	return buffer_ + reader_index_;
}

void StraightBuffer::IncWriterIndex(i32 size) {
	if (size <= 0) {
		size_reserve_ = 0;
		return;
	} else if (size > size_reserve_) {
		size = size_reserve_;
	}

	writer_index_ += size;
	size_reserve_ = 0;
}

void StraightBuffer::IncReaderIndex(i32 size) {
	if (size >= ReadableBytes()) {
		writer_index_ = 0;
		reader_index_ = 0;
	} else if (size > 0) {
		reader_index_ += size;
	}
}

i32 StraightBuffer::ReadableBytes() const {
	return writer_index_ - reader_index_;
}

i32 StraightBuffer::WritableBytes() const {
	return capacity_ - writer_index_ + reader_index_;
}

bool StraightBuffer::IsReadable() const {
	return writer_index_ - reader_index_ > 0;
}

bool StraightBuffer::IsWritable() const {
	return capacity_ - writer_index_ + reader_index_ > 0;
}

}