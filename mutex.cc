#include "mutex.h"

Mutex::Mutex()
{
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