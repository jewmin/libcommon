#include "service.h"

BaseService::BaseService(ILog * logger)
{
    this->_logger = logger;

    this->_loop = uv_loop_new();
    uv_async_init(this->_loop, &this->_stop_handle, BaseService::StopCallback);
    uv_prepare_init(this->_loop, &this->_msg_handle);
    this->_msg_handle.data = this;
}

BaseService::~BaseService()
{
    uv_walk(this->_loop, BaseService::WalkCallback, NULL);
    uv_run(this->_loop, UV_RUN_DEFAULT);
    uv_loop_delete(this->_loop);
}

void BaseService::Stop()
{
    if (this->_tid != 0)
    {
        uv_async_send(&this->_stop_handle);
        Super::Stop();
    }
}

void BaseService::PostMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5)
{
    AppMessage msg;
    msg.msg_id = msg_id;
    msg.param1 = param1;
    msg.param2 = param2;
    msg.param3 = param3;
    msg.param4 = param4;
    msg.param5 = param5;
    this->_msg_queue.Push(msg);
}

void BaseService::Run()
{
    uv_prepare_start(&this->_msg_handle, BaseService::MsgCallback);
    uv_run(this->_loop, UV_RUN_DEFAULT);
    uv_walk(this->_loop, BaseService::WalkCallback, NULL);
    uv_run(this->_loop, UV_RUN_DEFAULT);
}

void BaseService::OnTerminated()
{

}

void BaseService::OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5)
{

}

void BaseService::ProcessMsg()
{
    this->_msg_queue.Flush();

    size_t count = this->_msg_queue.size();
    for (size_t i = 0; i < count; i++)
    {
        AppMessage & msg = this->_msg_queue[i];
        this->OnRecvMsg(msg.msg_id, msg.param1, msg.param2, msg.param3, msg.param4, msg.param5);
    }
    
    this->_msg_queue.clear();
}

void BaseService::StopCallback(uv_async_t * handle)
{
    uv_stop(handle->loop);
}

void BaseService::WalkCallback(uv_handle_t * handle, void * arg)
{
    if (uv_is_closing(handle) == 0)
        uv_close(handle, NULL);
}

void BaseService::MsgCallback(uv_prepare_t * handle)
{
    BaseService * service = (BaseService *)handle->data;
    service->ProcessMsg();
}