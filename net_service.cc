#include "net_service.h"

NetService::NetService(ILog * logger)
{
    _logger = logger;
    _operation_queue.SetMutex(&_mutex);
}

NetService::~NetService()
{
    _operation_queue.Flush();
    _operation_queue.clear();
}

void NetService::Stop()
{
    NotifyOperation op;
    op.type = NetService::NOTIFY_STOP_LOOP;
    _operation_queue.Push(op);
    uv_async_send(&_async);
    BaseThread::Stop();
}

void NetService::Run()
{
    _loop = uv_default_loop();
    uv_async_init(_loop, &_async, NetService::OnNotify);
    _async.data = this;
    uv_run(_loop, UV_RUN_DEFAULT);
    uv_loop_close(_loop);
}

void NetService::OnTerminated()
{
    if (_logger)
        _logger->Info("NetService Terminated");
}

void NetService::OnNotify(uv_async_t * handle)
{
    NetService * service = (NetService *)handle->data;
    service->_operation_queue.Flush();
    for (size_t i = 0; i < service->_operation_queue.size(); i++)
    {
        NotifyOperation & operation = service->_operation_queue[i];
        if (operation.type == NetService::NOTIFY_STOP_LOOP)
        {
            uv_stop(service->_loop);
        }
        else if (operation.type == NetService::NOTIFY_CREATE_SERVER)
        {

        }
        else if (operation.type == NetService::NOTIFY_CREATE_CLIENT)
        {
            
        }
        else if (operation.type == NetService::NOTIFY_CONNECTION_WRITE)
        {
            
        }
    }
    service->_operation_queue.clear();
}

void NetService::OnAccept(uv_connect_t * req, int status)
{

}

void NetService::OnConnect(uv_connect_t * req, int status)
{

}

void NetService::OnAlloc(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{

}

void NetService::OnRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buf)
{

}

void NetService::OnWrite(uv_write_t * req, int status)
{

}

void NetService::OnTimer(uv_timer_t * handle)
{

}

void NetService::OnShutDown(uv_shutdown_t * req, int status)
{

}

void NetService::OnClose(uv_handle_t * handle)
{

}