#ifndef __LIB_COMMON_DOUBLE_BUFFER_H__
#define __LIB_COMMON_DOUBLE_BUFFER_H__

#include <list>
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
        Mutex::Owner lock(this->_mutex);

        this->_recv_buffer->push_back(data);
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
        Mutex::Owner lock(this->_mutex);

        std::list<T> * tmp_buffer = this->_recv_buffer;
        this->_recv_buffer = this->_proc_buffer;
        this->_proc_buffer = tmp_buffer;
    }

private:
    Mutex _mutex;
    std::list<T> _buffer1;
    std::list<T> _buffer2;
    std::list<T> * _proc_buffer;
    std::list<T> * _recv_buffer;
};

#endif