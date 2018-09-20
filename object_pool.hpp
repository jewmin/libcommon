#ifndef __LIBCOMMON_OBJECT_POOL_HPP__
#define __LIBCOMMON_OBJECT_POOL_HPP__

#include <new>
#include "common.h"
#include "vector.hpp"
#include "non_copy_able.hpp"

template<class T, int SIZE = 1024>
class ObjectPool : public NonCopyAble
{
public:
    ObjectPool() : total_object_count_(0), free_object_count_(0) {}

    virtual ~ObjectPool()
    {
        assert(total_object_count_ == free_object_count_);

        for (int i = free_list_.Count() - 1; i > -1; --i)
            free_list_[i]->~T();

        for (int i = chunk_list_.Count() - 1; i > -1; --i)
            jc_free(chunk_list_[i]);
    }

    virtual T * Allocate()
    {
        if (free_list_.Count() == 0)
        {
            T * new_chunk = static_cast<T *>(jc_calloc(SIZE, sizeof(T)));
            chunk_list_.Add(new_chunk);
            total_object_count_ += SIZE;
            free_list_.Reserve(total_object_count_);

            for (int i = 0; i < SIZE; ++i)
            {
                new(new_chunk)T();
                free_list_[i] = new_chunk;
                new_chunk++;
            }
            free_object_count_ = SIZE;
            free_list_.Trunc(free_object_count_);
        }

        assert(free_object_count_ == free_list_.Count());
        --free_object_count_;
        T * object = free_list_[free_object_count_];
        free_list_.Trunc(free_object_count_);
        return object;
    }

    void Release(T * object)
    {
        if (object)
        {
            free_list_.Add(object);
            ++free_object_count_;
        }
    }

    void ReleaseList(T * * list, int count)
    {
        if (list && count > 0)
        {
            free_list_.AddArray(list, count);
            free_object_count_ += count;
        }
    }

protected:
    BaseVector<T *> chunk_list_;
    BaseVector<T *> free_list_;
    int total_object_count_;
    int free_object_count_;
};

#endif