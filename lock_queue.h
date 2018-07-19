#ifndef __LIB_COMMON_LOCK_QUEUE_H__
#define __LIB_COMMON_LOCK_QUEUE_H__

#include "lock_vector.h"

template<typename T>
class LockQueue : public LockVector<T>
{
public:
    inline void Push(const T & data)
    {
        this->Lock();
        this->_append_vector.push_back(data);
        this->Unlock();
    }

    inline void Push(std::vector<T> & vec)
    {
        this->Lock();
        this->_append_vector.insert(this->_append_vector.end(), vec.begin(), vec.end());
        this->Unlock();
    }

    inline size_t GetAppendSize()
    {
        return this->_append_vector.size();
    }

    inline void Flush()
    {
        this->Lock();
        if (!this->_append_vector.empty())
        {
            insert(end(), this->_append_vector.begin(), this->_append_vector.end());
            this->_append_vector.clear();
        }
        this->Unlock();
    }

private:
    std::vector<T> _append_vector;
};

#endif