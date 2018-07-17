#ifndef __LIB_COMMON_SERVICE_H__
#define __LIB_COMMON_SERVICE_H__

#include "uv.h"
#include "thread.h"
#include "log.h"
#include "mutex.h"
#include "lock_queue.h"
#include "app_msg.h"

class BaseService : public BaseThread
{
    typedef BaseThread Super;

public:
    explicit BaseService(ILog * logger = NULL);
    virtual ~BaseService();
    virtual void Stop();
    void PostMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

protected:
    virtual void Run();
    virtual void OnTerminated();
    virtual void OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);
    void ProcessMsg();

    static void AsyncCallback(uv_async_t * handle);
    static void WalkCallback(uv_handle_t * handle, void * arg);
    static void MsgCallback(uv_async_t * handle);

protected:
    uv_loop_t * _loop;
    uv_async_t _stop_handle;
    uv_async_t _msg_handle;
    ILog * _logger;
    Mutex _lock;
    LockQueue<AppMessage> _msg_queue;
};

#endif