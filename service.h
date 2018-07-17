#ifndef __LIB_COMMON_SERVICE_H__
#define __LIB_COMMON_SERVICE_H__

#include "uv.h"
#include "thread.h"
#include "log.h"

class BaseService : public BaseThread
{
    typedef BaseThread Super;

public:
    explicit BaseService(ILog * logger = NULL);
    virtual ~BaseService();
    virtual void Stop();

protected:
    virtual void Run();
    virtual void OnTerminated();

    static void AsyncCallback(uv_async_t * handle);
    static void WalkCallback(uv_handle_t * handle, void * arg);

protected:
    uv_loop_t * _loop;
    uv_async_t _stop_handle;
    ILog * _logger;
};

#endif