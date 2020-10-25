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

#include "Buffer/BipBuffer.h"

namespace Common {

BipBuffer::BipBuffer() : offset_a_(0), size_a_(0), offset_b_(0), size_b_(0), offset_reserve_(0), size_reserve_(0) {
}

BipBuffer::~BipBuffer() {
}

i8 * BipBuffer::WritableBlock(i32 want_size, i32 & writable_size) {
	if (size_b_ > 0) {
		i32 free_space = GetBFreeSpace();
		if (free_space <= 0) {
			writable_size = 0;
			return nullptr;
		}

		if (free_space < want_size) {
			want_size = free_space;
		}
		size_reserve_ = writable_size = want_size;
		offset_reserve_ = offset_b_ + size_b_;
		return buffer_ + offset_reserve_;
	}

	i32 free_space = GetSpaceAfterA();
	if (free_space >= want_size) {
		size_reserve_ = writable_size = want_size;
		offset_reserve_ = offset_a_ + size_a_;
		return buffer_ + offset_reserve_;
	}

	if (free_space >= offset_a_) {
		if (free_space <= 0) {
			writable_size = 0;
			return nullptr;
		}

		size_reserve_ = writable_size = free_space;
		offset_reserve_ = offset_a_ + size_a_;
		return buffer_ + offset_reserve_;
	}

	if (offset_a_ < want_size) {
		want_size = offset_a_;
	}
	size_reserve_ = writable_size = want_size;
	offset_reserve_ = 0;
	return buffer_;
}

i8 * BipBuffer::ReadableBlock(i32 & readable_size) {
	if (size_a_ <= 0) {
		readable_size = 0;
		return nullptr;
	}

	readable_size = size_a_;
	return buffer_ + offset_a_;
}

void BipBuffer::IncWriterIndex(i32 size) {
	if (size <= 0) {
		offset_reserve_ = size_reserve_ = 0;
		return;
	} else if (size > size_reserve_) {
		size = size_reserve_;
	}

	if (0 == size_a_ && 0 == size_b_) {
		offset_a_ = offset_reserve_;
		size_a_ = size;
	} else if (offset_a_ + size_a_ == offset_reserve_) {
		size_a_ += size;
	} else {
		size_b_ += size;
	}
	offset_reserve_ = size_reserve_ = 0;
}

void BipBuffer::IncReaderIndex(i32 size) {
	if (size >= size_a_) {
		offset_a_ = offset_b_;
		size_a_ = size_b_;
		offset_b_ = size_b_ = 0;
	} else if (size > 0) {
		offset_a_ += size;
		size_a_ -= size;
	}
}

i32 BipBuffer::ReadableBytes() const {
	return size_a_ + size_b_;
}

i32 BipBuffer::WritableBytes() const {
	if (size_b_ > 0) {
		return GetBFreeSpace();
	} else {
		return std::max(GetBFreeSpace(), GetSpaceAfterA());
	}
}

bool BipBuffer::IsReadable() const {
	return size_a_ + size_b_ > 0;
}

bool BipBuffer::IsWritable() const {
	if (size_b_ > 0) {
		return GetBFreeSpace() > 0;
	} else {
		return std::max(GetBFreeSpace(), GetSpaceAfterA()) > 0;
	}
}

}