#ifndef __LIBCOMMON_MUTEX_H__
#define __LIBCOMMON_MUTEX_H__

#include "uv.h"
#include "non_copy_able.hpp"

class Mutex : public NonCopyAble {
public:
    explicit Mutex(bool recursive = true);
    ~Mutex();
    void Lock();
    int TryLock();
    void Unlock();

    class Guard : public NonCopyAble {
    public:
        explicit Guard(Mutex & lock);
        ~Guard();

    private:
        Mutex & lock_;
    };

private:
    uv_mutex_t lock_;
};

#endif