#ifndef __LIB_COMMON_QUEUE_H__
#define __LIB_COMMON_QUEUE_H__

#include "mutex.h"
#include <vector>

template<typename T>
class Queue : public std::vector<T>
{
public:
    explicit Queue(Mutex * mutex = nullptr)
    {
        _mutex = mutex;
    }

    inline Mutex * GetMutex()
    {
        return _mutex;
    }

    inline Mutex * SetMutex(Mutex * mutex)
    {
        Mutex * old_mutex = _mutex;
        _mutex = mutex;
        return old_mutex;
    }

    inline void Lock()
    {
        if (_mutex)
            _mutex->Lock();
    }

    inline void Unlock()
    {
        if (_mutex)
            _mutex->Unlock();
    }

    inline void Push(const T & data)
    {
        Lock();
        _append.push_back(data);
        Unlock();
    }

    inline void Push(std::vector<T> & vec)
    {
        Lock();
        _append.insert(_append.end(), vec.begin(), vec.end());
		Unlock();
    }

	inline size_t AppendSize()
	{
		return _append.size();
	}

	inline void Flush()
	{
		Lock();
		if (!_append.empty())
		{
			insert(end(), _append.begin(), _append.end());
			_append.clear();
		}
		Unlock();
	}

private:
    Mutex * _mutex;
    std::vector<T> _append;
};

#endif