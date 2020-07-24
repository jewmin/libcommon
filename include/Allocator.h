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

#ifndef Common_Allocator_INCLUDED
#define Common_Allocator_INCLUDED

#include "Common.h"

// 定义内存分配原子函数
typedef void * (*jc_malloc_func)(std::size_t size);
typedef void * (*jc_realloc_func)(void * ptr, std::size_t size);
typedef void * (*jc_calloc_func)(std::size_t count, std::size_t size);
typedef void (*jc_free_func)(void * ptr);

#ifdef __cplusplus
extern "C" {
#endif

// 定义基本内存分配函数，所有内存分配都走这里
COMMON_EXTERN void * jc_malloc(std::size_t size);
COMMON_EXTERN void * jc_realloc(void * ptr, std::size_t size);
COMMON_EXTERN void * jc_calloc(std::size_t count, std::size_t size);
COMMON_EXTERN void jc_free(void * ptr);
COMMON_EXTERN bool jc_replace_allocator(jc_malloc_func malloc_func, jc_realloc_func realloc_func, jc_calloc_func calloc_func, jc_free_func free_func);

#ifdef __cplusplus
}
#endif

#endif