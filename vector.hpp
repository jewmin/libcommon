#ifndef __LIBCOMMON_VECTOR_HPP__
#define __LIBCOMMON_VECTOR_HPP__

#include "common.h"
#include "non_copy_able.hpp"

template<typename T, int SIZE = 10>
class BaseVector : public NonCopyAble {
public:
    BaseVector() : data_(nullptr), capacity_(0), count_(0) {

    }

    virtual ~BaseVector() {
        Empty();
    }

    inline int Capacity() const {
        return capacity_;
    }

    inline int Count() const {
        return count_;
    }

    inline void Clear() {
        count_ = 0;
    }

    inline void Trunc(const int count) {
        assert(count > -1 && count <= capacity_);
        count_ = count;
    }

    inline const T & Get(const int index) const {
        assert(index > -1 && index < count_);
        return data_[index];
    }

    inline void Set(const int index, const T & data) {
        assert(index > -1 && index < count_);
        data_[index] = data;
    }

    inline operator T * () const {
        return data_;
    }

    // ������λ�ò�������
    inline void Insert(const int index, const T & data) {
        assert(index > -1 && index <= count_);

        if (count_ >= capacity_) {
            Reserve(capacity_ > 0 ? capacity_ << 1 : SIZE);
        }

        if (index < count_) {
            memmove(data_ + index + 1, data_ + index, sizeof(T) * (count_ - index));
        }

        data_[index] = data;
        ++count_;
    }

    // �������ݣ���������
    inline int Add(const T & data) {
        if (count_ >= capacity_) {
            Reserve(capacity_ > 0 ? capacity_ << 1 : SIZE);
        }

        data_[count_] = data;
        ++count_;
        return count_ - 1;
    }

    // ��������
    inline void AddVector(const BaseVector<T> & vec) {
        AddArray(static_cast<T *>(vec), vec.Count());
    }

    // ��������
    inline void AddArray(const T * data, const int count) {
        if (count_ + count > capacity_) {
            Reserve(count_ + count);
        }

        memcpy(data_ + count_, data, sizeof(T) * count);
        count_ += count;
    }

    // ��������ɾ������
    inline void Remove(const int index) {
        assert(index > -1 && index < count_);
        Remove(index, 1);
    }

    // ɾ�������������count������
    inline void Remove(const int index, const int count) {
        assert(index > -1 && index + count <= count_);
        if (count > 0) {
            memmove(data_ + index, data_ + index + count, sizeof(T) * (count_ - index - count));
            count_ -= count;
        }
    }

    // �������ݣ��ҵ��򷵻����������򷵻�-1
    int Find(const T & data) const {
        for (int i = count_ - 1; i > -1; --i) {
            if (data == data_[i]) return i;
        }

        return -1;
    }

    // ������ݣ����ͷ��ڴ�
    virtual void Empty() {
        Clear();
        capacity_ = 0;

        if (data_) {
            jc_free(data_);
            data_ = nullptr;
        }
    }

    // ���·����ڴ�
    virtual void Reserve(const int count) {
        if (count > count_ && count != capacity_) {
            capacity_ = count;
            data_ = static_cast<T *>(jc_realloc(data_, sizeof(T) * count));
        }
    }

protected:
    T * data_;
    int capacity_;
    int count_;
};

#endif