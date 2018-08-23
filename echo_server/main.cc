#include <signal.h>
#include "common.h"
#include "test_log.h"
#include "exception.h"
#include "echo_server.h"

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
    uv_replace_allocator(jc_malloc, jc_realloc, jc_calloc, jc_free);

    TestLog log("echo_server.log");
    log.Start();

    try
    {
        uv_loop_t * loop = uv_default_loop();

        EchoServer server("Welcome to echo server! What are you doing now?", 10, 10, 1500, &log);

        server.SetNoDelay(true);
        server.SetKeepAlive(60);
        
        server.Open("::", 6789);

        server.StartAcceptingConnections();

        uv_signal_t sig;
        uv_signal_init(loop, &sig);
        uv_signal_start(&sig, signal_handler, SIGINT);
        uv_signal_start(&sig, signal_handler, SIGTERM);

        uv_run(loop, UV_RUN_DEFAULT);

        server.WaitForShutdownToComplete();

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