#ifndef __LIB_COMMON_LOCK_VECTOR_H__
#define __LIB_COMMON_LOCK_VECTOR_H__

#include <vector>
#include "mutex.h"

template<typename T>
class LockVector : public std::vector<T>
{
public:
    explicit LockVector(Mutex * mutex = NULL)
    {
        this->_mutex = mutex;
    }

    inline Mutex * GetMutex()
    {
        return this->_mutex;
    }

    inline Mutex * SetMutex(Mutex * mutex)
    {
        Mutex * old_mutex = this->_mutex;
        this->_mutex = mutex;
        return old_mutex;
    }

    inline void Lock()
    {
        if (this->_mutex)
            this->_mutex->Lock();
    }

    inline void Unlock()
    {
        if (this->_mutex)
            this->_mutex->Unlock();
    }

private:
    Mutex _mutex;
};

#endif