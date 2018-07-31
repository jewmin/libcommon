#include "echo_client.h"
#include "test_log.h"
#include "exception.h"
#include "signal.h"

void close_cb(uv_handle_t * handle, void * arg)
{
    if (uv_is_closing(handle) == 0)
        uv_close(handle, NULL);
}

void signal_handler(uv_signal_t * handle, int signum)
{
    uv_signal_stop(handle);
    uv_stop(handle->loop);
}

int main(int argc, const char * * argv)
{
    TestLog log("echo_client.log");
    log.Start();

    try
    {
        uv_loop_t * loop = uv_default_loop();

        EchoClient client(10, 1024, &log);
        
        client.ConnectTo("127.0.0.1", 6789);

        client.StartConnections();

        uv_signal_t sig;
        uv_signal_init(loop, &sig);
        uv_signal_start(&sig, signal_handler, SIGINT);

        uv_run(loop, UV_RUN_DEFAULT);

        client.WaitForShutdownToComplete();

        uv_walk(loop, close_cb, NULL);

        uv_run(loop, UV_RUN_DEFAULT);

        uv_loop_close(loop);
    }
    catch (const BaseException & ex)
    {
        log.Error("Exception: %s - %s", ex.Where().c_str(), ex.Message().c_str());
    }
    catch (...)
    {
        log.Error("Unexpected exception");
    }

    log.Stop();

    return 0;
}