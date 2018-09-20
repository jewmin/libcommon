#ifndef __LIBCOMMON_SERVICE_H__
#define __LIBCOMMON_SERVICE_H__

#include "uv.h"
#include "logger.h"
#include "thread.h"
#include "app_msg.h"
#include "queue.hpp"

class BaseService : public BaseThread
{
    typedef BaseThread Super;

public:
    explicit BaseService(Logger * logger = NULL);
    virtual ~BaseService();
    virtual void Stop();
    void PostMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

protected:
    virtual void Run();
    virtual void OnTerminated();
    virtual void OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);
    void ProcessMsg();

    static void StopCallback(uv_async_t * handle);
    static void WalkCallback(uv_handle_t * handle, void * arg);
    static void MsgCallback(uv_prepare_t * handle);

protected:
    uv_loop_t * _loop;
    uv_async_t _stop_handle;
    uv_prepare_t _msg_handle;
    uv_async_t _msg_notify_handle;
    
    Logger * _logger;
    LockQueue<AppMessage> _msg_queue;
};

#endif