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

#ifndef Common_Common_INCLUDED
#define Common_Common_INCLUDED

// stl标准库头文件
#include <new>
#include <set>
#include <map>
#include <list>
#include <ctime>
#include <atomic>
#include <vector>
#include <string>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <cctype>

// C头文件
#include <assert.h>
#ifdef _WIN32
#	include <io.h>
#	include <process.h>
#else
#	include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

// 标准描述符
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

// 定义标准类型，内部统一使用
typedef char				i8;
typedef short				i16;
typedef int					i32;
typedef long long			i64;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

// 动态库
#ifdef _WIN32
	/* Windows - set up dll import/export decorators. */
#	if defined(BUILDING_COMMON_SHARED)
		/* Building shared library. */
#		define COMMON_EXTERN __declspec(dllexport)
#	elif defined(USING_COMMON_SHARED)
		/* Using shared library. */
#		define COMMON_EXTERN __declspec(dllimport)
#	else
		/* Building static library. */
#		define COMMON_EXTERN /* nothing */
#	endif
#elif __GNUC__ >= 4
#	define COMMON_EXTERN __attribute__((visibility("default")))
#else
#	define COMMON_EXTERN /* nothing */
#endif

#endif