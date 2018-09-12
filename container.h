#ifndef __LIB_COMMON_CONTAINER_H__
#define __LIB_COMMON_CONTAINER_H__

#include <list>
#include <vector>
#include "mutex.h"

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