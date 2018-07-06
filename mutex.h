#ifndef __LIB_COMMON_MUTEX_H__
#define __LIB_COMMON_MUTEX_H__

#include "uv.h"

class Mutex
{
public:
    explicit Mutex(bool recursive = false);
    virtual ~Mutex();
    void Lock();
    void Unlock();
    int TryLock();

private:
    uv_mutex_t _mutex;
};

#endif