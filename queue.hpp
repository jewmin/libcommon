#ifndef __LIBCOMMON_QUEUE_HPP__
#define __LIBCOMMON_QUEUE_HPP__

#include "mutex.h"
#include "vector.hpp"

template<typename T, int SIZE = 10>
class LockQueue : public BaseVector<T, SIZE>
{
public:
    inline void Push(const T & data)
    {
        lock_.Lock();
        append_vec_.Add(data);
        lock_.Unlock();
    }

    inline void PushList(const BaseVector<T> & vec)
    {
        lock_.Lock();
        append_vec_.AddArray(static_cast<T *>(vec), vec.Count());
        lock_.Unlock();
    }

    inline void PushArray(T * data, int count)
    {
        lock_.Lock();
        append_vec_.AddArray(data, count);
        lock_.Unlock();
    }

    inline int AppendCount() { return append_vec_.Count(); }

    inline void Flush()
    {
        lock_.Lock();
        if (append_vec_.Count() > 0)
        {
            AddArray(static_cast<T *>(append_vec_), append_vec_.Count());
            append_vec_.Clear();
        }
        lock_.Unlock();
    }

private:
    Mutex lock_;
    BaseVector<T> append_vec_;
};

#endif