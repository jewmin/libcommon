#include <signal.h>
#include "echo_server.h"
#include "file_logger.h"
#include "event_loop_thread.h"

void signal_handler(EventLoop * loop, int signum) {
    if (loop->log()) {
        loop->log()->LogInfo("signal_handler: %d", signum);
    }
    loop->Quit();
}

int main(int argc, const char * * argv) {
    uv_replace_allocator(jc_malloc, jc_realloc, jc_calloc, jc_free);

    FileLogger logger("echo_client.log");
    logger.InitLogger(Logger::Info);

    EventLoop main_loop(&logger);
    main_loop.StartSignal(SIGINT, std::bind(&signal_handler, &main_loop, std::placeholders::_1));

    EventLoopThread * io_thread = new EventLoopThread(&logger);
    EventLoop * io_loop = io_thread->StartLoop();
    EchoServer server("welcome to echo server", io_loop);
    server.Listen("::", 6789);
    
    main_loop.Loop();
    
    server.Shutdown();

    delete io_thread;

    logger.DeInitLogger();

    return 0;
}