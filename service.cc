#include "service.h"

BaseService::BaseService(ILog * logger)
{
    _logger = logger;
    _loop = uv_loop_new();
}

BaseService::~BaseService()
{
    uv_loop_delete(_loop);
}

void BaseService::Stop()
{
    uv_async_send(&_stop_handle);
    Super::Stop();
}

void BaseService::Run()
{
    uv_async_init(_loop, &_stop_handle, BaseService::AsyncCallback);
    uv_run(_loop, UV_RUN_DEFAULT);
    uv_walk(_loop, BaseService::WalkCallback, NULL);
    uv_run(_loop, UV_RUN_DEFAULT);
}

void BaseService::OnTerminated()
{
    if (_logger)
        _logger->Info("BaseService Terminated");
}

void BaseService::AsyncCallback(uv_async_t * handle)
{
    uv_stop(handle->loop);
}

void BaseService::WalkCallback(uv_handle_t * handle, void * arg)
{
    if (uv_is_closing(handle) == 0)
        uv_close(handle, NULL);
}