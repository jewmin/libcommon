#include "tcp_test.h"
#include "gtest/gtest.h"

TEST(TcpTest, tcp_close)
{
    MockLog log;
    MockTcpServer server(&log);
    server.Listen("0.0.0.0", 5678);

    MockTcpClient client1(&log), client2(&log);
    client1.Connect("127.0.0.1", 5678);
    client2.Connect("127.0.0.1", 5678);

    Sleep(10000);

    client1.Stop();
    client2.Stop();
    server.Stop();
}

TEST(TcpTest, tcp_inherit_ipv6)
{
	MockLog log;
	MockNeedTcpServer server(&log);
	server.Listen("::", 5678);

	MockNeedTcpClient * clients[10];
	for (int i = 0; i < 10; i++)
	{
		clients[i] = new MockNeedTcpClient(&log);
		clients[i]->Connect("::1", 5678);
	}

	Sleep(10);

	for (int i = 0; i < 10; i++)
	{
		clients[i]->Stop();
		delete clients[i];
	}
	
	server.Stop();
}