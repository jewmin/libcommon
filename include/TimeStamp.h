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

#ifndef Common_TimeStamp_INCLUDED
#define Common_TimeStamp_INCLUDED

#include "CObject.h"

namespace Common {

class COMMON_EXTERN TimeStamp : public CObject {
public:
	TimeStamp();
	TimeStamp(i64 seconds, i64 micro_seconds = 0);
	TimeStamp(const TimeStamp & rhs);
	TimeStamp & operator=(const TimeStamp & rhs);
	virtual ~TimeStamp();

	i64 GetSeconds() const;
	i64 GetMilliSeconds() const;
	i64 GetMicroSeconds() const;

private:
	i64 seconds_;
	i64 micro_seconds_;
};

inline i64 TimeStamp::GetSeconds() const {
	return seconds_;
}

inline i64 TimeStamp::GetMilliSeconds() const {
	return micro_seconds_ / 1000;
}

inline i64 TimeStamp::GetMicroSeconds() const {
	return micro_seconds_;
}

}

#endif