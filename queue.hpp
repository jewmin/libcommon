#ifndef __LIBCOMMON_QUEUE_HPP__
#define __LIBCOMMON_QUEUE_HPP__

#include "mutex.h"
#include "vector.hpp"

template<typename T, int SIZE = 10>
class LockQueue : public BaseVector<T, SIZE> {
public:
    inline void Push(const T & data) {
        Mutex::Guard guard(lock_);
        append_vec_.Add(data);
    }

    inline void PushList(const BaseVector<T> & vec) {
        Mutex::Guard guard(lock_);
        append_vec_.AddArray(static_cast<T *>(vec), vec.Count());
    }

    inline void PushArray(T * data, int count) {
        Mutex::Guard guard(lock_);
        append_vec_.AddArray(data, count);
    }

    inline int AppendCount() {
        return append_vec_.Count();
    }

    inline void Flush() {
        Mutex::Guard guard(lock_);
        if (append_vec_.Count() > 0) {
            AddArray(static_cast<T *>(append_vec_), append_vec_.Count());
            append_vec_.Clear();
        }
    }

private:
    Mutex lock_;
    BaseVector<T> append_vec_;
};

#endif