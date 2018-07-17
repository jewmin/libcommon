#include "service.h"

BaseService::BaseService(ILog * logger)
{
    this->_logger = logger;
    this->_loop = uv_loop_new();
}

BaseService::~BaseService()
{
    uv_loop_delete(this->_loop);
}

void BaseService::Stop()
{
    uv_async_send(&this->_stop_handle);
    Super::Stop();
}

void BaseService::Run()
{
    uv_async_init(this->_loop, &this->_stop_handle, BaseService::AsyncCallback);
    uv_run(this->_loop, UV_RUN_DEFAULT);
    uv_walk(this->_loop, BaseService::WalkCallback, NULL);
    uv_run(this->_loop, UV_RUN_DEFAULT);
}

void BaseService::OnTerminated()
{
    if (this->_logger)
        this->_logger->Info("BaseService Terminated");
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