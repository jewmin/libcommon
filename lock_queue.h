#ifndef __LIB_COMMON_LOCK_QUEUE_H__
#define __LIB_COMMON_LOCK_QUEUE_H__

#include <vector>
#include "mutex.h"

template<typename T>
class LockQueue : public std::vector<T>
{
public:
    inline void Push(const T & data)
    {
        this->_mutex.Lock();
        this->_append_vector.push_back(data);
        this->_mutex.Unlock();
    }

    inline void Push(std::vector<T> & vec)
    {
        this->_mutex.Lock();
        this->_append_vector.insert(this->_append_vector.end(), vec.begin(), vec.end());
        this->_mutex.Unlock();
    }

    inline size_t GetAppendSize()
    {
        return this->_append_vector.size();
    }

    inline void Flush()
    {
        this->_mutex.Lock();
        if (!this->_append_vector.empty())
        {
            insert(end(), this->_append_vector.begin(), this->_append_vector.end());
            this->_append_vector.clear();
        }
        this->_mutex.Unlock();
    }

private:
    Mutex _mutex;
    std::vector<T> _append_vector;
};

#endif