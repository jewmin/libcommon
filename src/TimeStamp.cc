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

TimeStamp::TimeStamp(const TimeStamp & rhs) : seconds_(rhs.seconds_), micro_seconds_(rhs.micro_seconds_) {
}

TimeStamp & TimeStamp::operator=(const TimeStamp & rhs) {
	if (this != &rhs) {
		seconds_ = rhs.seconds_;
		micro_seconds_ = rhs.micro_seconds_;
	}
	return *this;
}

TimeStamp::~TimeStamp() {
}

}