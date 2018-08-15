#ifndef __LIB_COMMON_COMMON_H__
#define __LIB_COMMON_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

/* 定义内存分配 */
#ifdef USE_JEMALLOC
#include "jemalloc/jemalloc.h"
#define jc_malloc je_malloc
#define jc_realloc je_realloc
#define jc_calloc je_calloc
#define jc_free je_free
#else
#define jc_malloc malloc
#define jc_realloc realloc
#define jc_calloc calloc
#define jc_free free
#endif

#ifdef _MSC_VER
#define jc_sleep(x) Sleep(x)
#else
#define jc_sleep(x) usleep((x) * 1000)
#endif

#endif