#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#include <string>

std::string ToHex(uint8_t c);

std::string DumpData(const uint8_t * const data, size_t data_length, size_t line_length = 0);

#endif