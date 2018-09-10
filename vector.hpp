#ifndef __LIBCOMMON_VECTOR_HPP__
#define __LIBCOMMON_VECTOR_HPP__

#include "common.h"
#include "non_copy_able.hpp"

template<typename T, int SIZE = 10>
class BaseVector : public NonCopyAble
{
public:
    BaseVector() : data_(nullptr), capacity_(0), size_(0) {}
    virtual ~BaseVector() { Empty(); }

    inline int Capacity() const { return capacity_; }
    inline int Count() const { return size_; }
    inline void Clear() { size_ = 0; }

    inline void Trunc(const int count)
    {
        assert(count > -1 && count <= capacity_);
        size_ = count;
    }

    inline const T & Get(const int index) const
    {
        assert(index > -1 && index < size_);
        return data_[index];
    }

    inline void Set(const int index, const T & data)
    {
        assert(index > -1 && index < size_);
        data_[index] = data;
    }

    inline operator T * () const { return data_; }

    // 在索引位置插入数据
    inline void Insert(const int index, const T & data)
    {
        assert(index > -1 && index <= size_);

        if (size_ >= capacity_)
            Reserve(capacity_ > 0 ? capacity_ << 1 : SIZE);

        if (index < size_)
            memmove(data_ + index + 1, data_ + index, sizeof(T) * (size_ - index));

        data_[index] = data;
        size_++;
    }

    // 插入数据，返回索引
    inline int Add(const T & data)
    {
        if (size_ >= capacity_)
            Reserve(capacity_ > 0 ? capacity_ << 1 : SIZE);

        data_[size_] = data;
        size_++;
        return size_ - 1;
    }

    // 插入数组
    inline void AddVector(const BaseVector<T> & vec)
    {
        AddArray(static_cast<T *>(vec), vec.Count());
    }

    // 插入数组
    inline void AddArray(const T * data, const int count)
    {
        if (size_ + count > capacity_)
            Reserve(size_ + count);

        memcpy(data_ + size_, data, sizeof(T) * count);
        size_ += count;
    }

    // 根据索引删除数据
    inline void Remove(const int index)
    {
        assert(index > -1 && index < size_);
        Remove(index, 1);
    }

    // 删除从索引处起的count个数据
    inline void Remove(const int index, const int count)
    {
        assert(index > -1 && index + count <= size_);
        
        if (count > 0)
        {
            memcpy(data_ + index, data_ + index + count, sizeof(T) * (size_ - index - count));
            size_ -= count;
        }
    }

    // 查找数据，找到则返回索引，否则返回-1
    int Find(const T & data) const
    {
        for (int i = size_ - 1; i > -1; --i)
            if (data_[i] == data) return i;

        return -1;
    }

    // 清空数据，并释放内存
    virtual void Empty()
    {
        Clear();
        capacity_ = 0;

        if (data_)
        {
            jc_free(data_);
            data_ = nullptr;
        }
    }

    // 重新分配内存
    virtual void Reserve(const int count)
    {
        if (count > size_ && count != capacity_)
        {
            capacity_ = count;
            data_ = static_cast<T *>(jc_realloc(data_, sizeof(T) * count));
        }
    }

protected:
    T * data_;
    int capacity_;
    int size_;
};

#endif