#include "LocalTime.h"

#ifdef _WIN32
void jc_localtime(const std::time_t * time, std::tm * tm) {
	localtime_s(tm, time);
}
#else
void jc_localtime(const std::time_t * time, std::tm * tm) {
	localtime_r(time, tm);
}
#endif