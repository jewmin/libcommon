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

#ifndef Common_Buffer_BipBuffer_INCLUDED
#define Common_Buffer_BipBuffer_INCLUDED

#include "Buffer/CBuffer.h"

namespace Common {

class COMMON_EXTERN BipBuffer : public CBuffer {
public:
	BipBuffer();
	virtual ~BipBuffer();

	virtual i8 * WritableBlock(i32 want_size, i32 & writable_size) override;
	virtual i8 * ReadableBlock(i32 & readable_size) override;
	virtual void IncWriterIndex(i32 size) override;
	virtual void IncReaderIndex(i32 size) override;
	virtual i32 ReadableBytes() const override;
	virtual i32 WritableBytes() const override;
	virtual bool IsReadable() const override;
	virtual bool IsWritable() const override;

protected:
	i32 GetSpaceAfterA() const;
	i32 GetBFreeSpace() const;

private:
	i32 offset_a_;
	i32 size_a_;
	i32 offset_b_;
	i32 size_b_;
	i32 offset_reserve_;
	i32 size_reserve_;
};

inline i32 BipBuffer::GetSpaceAfterA() const {
	return capacity_ - offset_a_ - size_a_;
}

inline i32 BipBuffer::GetBFreeSpace() const {
	return offset_a_ - offset_b_ - size_b_;
}

}

#endif