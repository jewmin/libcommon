#include <vector>
#include <signal.h>

#include "get_opt.h"
#include "echo_client.h"
#include "file_logger.h"
#include "event_loop_thread.h"

void signal_handler(EventLoop * loop, int signum) {
    if (loop->log()) {
        loop->log()->LogInfo("signal_handler: %d", signum);
    }
    loop->Quit();
}

int main(int argc, char * * argv) {
    uv_replace_allocator(jc_malloc, jc_realloc, jc_calloc, jc_free);

    int count = 1;
    char host[128] = "127.0.0.1";
    int port = 6789;
    struct jc_option longopts[] = {
        { "count",  jc_required_argument, &count, 1 },
        { "host",	jc_required_argument, nullptr, 'h' },
        { "port",	jc_required_argument, &port, 'p' },
        { nullptr, 0, nullptr, 0 }
    };

    char c;
    while ((c = static_cast<char>(jc_getopt_long_only(argc, argv, ":f:", longopts, nullptr))) != -1) {
        switch (c)
        {
        case 'h':
            STRNCPY_S(host, jc_optarg);
            break;

        case 0:
            break;

        default:
            printf("Usage: %s [--host <hostname>] [--port <port>] [--count <client-count>]\n", argv[0]);
            return 0;
        }
    }

    FileLogger logger("echo_client.log");
    logger.InitLogger(Logger::Info);

    EventLoop main_loop(&logger);
    main_loop.StartSignal(SIGINT, std::bind(&signal_handler, &main_loop, std::placeholders::_1));
    main_loop.StartSignal(SIGTERM, std::bind(&signal_handler, &main_loop, std::placeholders::_1));

    EventLoopThread * io_thread = new EventLoopThread(&logger);
    EventLoop * io_loop = io_thread->StartLoop();
    
    std::vector<EchoClient *> client_list;
    client_list.reserve(count);
    for (int i = 0; i < count; ++i) {
        EchoClient * client = new EchoClient(io_loop);
        client->Connect(host, port);
        client_list.push_back(client);
    }
    
    main_loop.Loop();
    
    for (int i = 0; i < count; ++i) {
        client_list[i]->Shutdown();
    }

    delete io_thread;
    for (int i = 0; i < count; ++i) {
        delete client_list[i];
    }
    client_list.clear();

    logger.DeInitLogger();

    return 0;
}