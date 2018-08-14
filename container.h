#ifndef __LIB_COMMON_CONTAINER_H__
#define __LIB_COMMON_CONTAINER_H__

#include <list>
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

    inline size_t GetAppendedSize()
    {
        return this->_append_vector.size();
    }

    inline void Flush()
    {
        this->_mutex.Lock();
        if (!this->_append_vector.empty())
        {
            this->insert(this->end(), this->_append_vector.begin(), this->_append_vector.end());
            this->_append_vector.clear();
        }
        this->_mutex.Unlock();
    }

private:
    Mutex _mutex;
    std::vector<T> _append_vector;
};


template<typename T>
class DoubleQueue
{
public:
    DoubleQueue()
    {
        this->_proc_vector = &this->_vector1;
        this->_recv_vector = &this->_vector2;
    }

    inline void Push(const T & data)
    {
        this->_mutex.Lock();
        this->_recv_vector->push_back(data);
        this->_mutex.Unlock();
    }

    inline void Push(std::vector<T> & vec)
    {
        this->_mutex.Lock();
        this->_recv_vector->insert(this->_recv_vector->end(), vec.begin(), vec.end());
        this->_mutex.Unlock();
    }

    inline void Flush()
    {
        if (this->_proc_vector->empty() && !this->_recv_vector->empty())
            this->Swap();
    }

    inline void clear() { this->_proc_vector->clear(); }

    inline size_t size() { return this->_proc_vector->size(); }

    inline T & operator [] (int32_t index) { return this->_proc_vector->operator[](index); }

protected:
    inline void Swap()
    {
        this->_mutex.Lock();
        std::vector<T> * vec = this->_recv_vector;
        this->_recv_vector = this->_proc_vector;
        this->_proc_vector = vec;
        this->_mutex.Unlock();
    }

private:
    Mutex _mutex;
    std::vector<T> _vector1;
    std::vector<T> _vector2;
    std::vector<T> * _proc_vector;
    std::vector<T> * _recv_vector;
};


template<typename T>
class DoubleBuffer
{
public:
    DoubleBuffer()
    {
        this->_proc_buffer = &this->_buffer1;
        this->_recv_buffer = &this->_buffer2;
    }

    inline void Push(const T & data)
    {
        this->_mutex.Lock();
        this->_recv_buffer->push_back(data);
        this->_mutex.Unlock();
    }

    inline T Pop()
    {
        if (this->_proc_buffer->empty())
            this->Swap();

        T data = this->_proc_buffer->front();
        this->_proc_buffer->pop_front();
        return data;
    }

    inline T Front()
    {
        if (this->_proc_buffer->empty())
            this->Swap();

        return this->_proc_buffer->front();
    }

    inline void PopFront()
    {
        if (this->_proc_buffer->empty())
            this->Swap();

        this->_proc_buffer->pop_front();
    }

    inline bool Empty() { return this->_proc_buffer->empty() && this->_recv_buffer->empty(); }

    inline size_t Size() { return this->_proc_buffer->size() + this->_recv_buffer->size(); }

protected:
    inline void Swap()
    {
        this->_mutex.Lock();
        std::list<T> * tmp_buffer = this->_recv_buffer;
        this->_recv_buffer = this->_proc_buffer;
        this->_proc_buffer = tmp_buffer;
        this->_mutex.Unlock();
    }

private:
    Mutex _mutex;
    std::list<T> _buffer1;
    std::list<T> _buffer2;
    std::list<T> * _proc_buffer;
    std::list<T> * _recv_buffer;
};

#endif