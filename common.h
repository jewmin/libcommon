#ifndef __LIB_COMMON_COMMON_H__
#define __LIB_COMMON_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>

/* 定义内存分配 */
#define jc_malloc malloc
#define jc_realloc realloc
#define jc_calloc calloc
#define jc_free free

#ifdef _MSC_VER
#define jc_sleep(x) Sleep(x)
#else
#define jc_sleep(x) usleep((x) * 1000)
#endif

#endif