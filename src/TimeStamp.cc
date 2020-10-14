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

#include "TimeStamp.h"

namespace Common {

TimeStamp::TimeStamp() {
	std::chrono::nanoseconds now = std::chrono::system_clock::now().time_since_epoch();
	seconds_ = std::chrono::duration_cast<std::chrono::seconds>(now).count();
	now -= std::chrono::seconds(seconds_);
	micro_seconds_ = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
}

TimeStamp::TimeStamp(i64 seconds, i64 micro_seconds) : seconds_(seconds), micro_seconds_(micro_seconds) {
}

TimeStamp::TimeStamp(const TimeStamp & other) : seconds_(other.seconds_), micro_seconds_(other.micro_seconds_) {
}

TimeStamp & TimeStamp::operator=(const TimeStamp & other) {
	if (this != std::addressof(other)) {
		seconds_ = other.seconds_;
		micro_seconds_ = other.micro_seconds_;
	}
	return *this;
}

TimeStamp::~TimeStamp() {
}

}