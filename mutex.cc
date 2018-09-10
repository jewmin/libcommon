#include "mutex.h"

Mutex::Mutex(bool recursive)
{
    if (recursive)
        uv_mutex_init_recursive(&lock_);
    else
        uv_mutex_init(&lock_);
}

Mutex::~Mutex()
{
    uv_mutex_destroy(&lock_);
}

void Mutex::Lock()
{
    uv_mutex_lock(&lock_);
}

void Mutex::Unlock()
{
    uv_mutex_unlock(&lock_);
}

int Mutex::TryLock()
{
    return uv_mutex_trylock(&lock_);
}

Mutex::Guard::Guard(Mutex & lock)
    : lock_(lock)
{
    lock_.Lock();
}

Mutex::Guard::~Guard()
{
    lock_.Unlock();
}