#include "mutex.h"

Mutex::Mutex(bool recursive)
{
    if (recursive)
        uv_mutex_init_recursive(&_mutex);
    else
        uv_mutex_init(&_mutex);
}

Mutex::~Mutex()
{
    uv_mutex_destroy(&_mutex);
}

void Mutex::Lock()
{
    uv_mutex_lock(&_mutex);
}

void Mutex::Unlock()
{
    uv_mutex_unlock(&_mutex);
}

int Mutex::TryLock()
{
    return uv_mutex_trylock(&_mutex);
}

Mutex::Owner::Owner(Mutex & mutex)
    : _mutex(mutex)
{
    this->_mutex.Lock();
}

Mutex::Owner::~Owner()
{
    this->_mutex.Unlock();
}