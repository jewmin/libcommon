#ifndef __LIB_COMMON_MUTEX_H__
#define __LIB_COMMON_MUTEX_H__

#include "uv.h"

class Mutex
{
public:
    class Owner
    {
    public:
        explicit Owner(Mutex & mutex);
        ~Owner();

    private:
        /*
         * No copies do not implement
         */
        Owner(const Owner & rhs);
        Owner & operator =(const Owner & rhs);

    private:
        Mutex & _mutex;
    };

    explicit Mutex(bool recursive = false);
    virtual ~Mutex();
    void Lock();
    void Unlock();
    int TryLock();

private:
    /*
     * No copies do not implement
     */
    Mutex(const Mutex & rhs);
    Mutex & operator =(const Mutex & rhs);

private:
    uv_mutex_t _mutex;
};

#endif