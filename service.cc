#include "service.h"

BaseService::BaseService(ILog * logger)
{
    _logger = logger;
}

BaseService::~BaseService()
{

}

void BaseService::Stop()
{
    uv_async_send(&_stop_handle);
    BaseThread::Stop();
}

void BaseService::Run()
{
    uv_loop_t * loop = uv_loop_new();
    uv_async_init(loop, &_stop_handle, BaseService::AsyncCallback);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_walk(loop, BaseService::WalkCallback, NULL);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_delete(loop);
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