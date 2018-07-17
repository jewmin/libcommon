#ifndef __UNIT_TEST_TCP_TEST_H__
#define __UNIT_TEST_TCP_TEST_H__

#include "gmock/gmock.h"
#include "log.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "tcp_connection.h"
#include "mutex.h"

class MockLog : public ILog
{
	Mutex _lock;
public:
    MockLog() {}
    ~MockLog() {}

    inline const char * format_msg(const char * fmt, va_list args)
    {
        static char buf[1024];
        int ret = vsnprintf(buf, sizeof(buf), fmt, args);
        if (ret >= sizeof(buf)) buf[sizeof(buf) - 1] = 0;
        else buf[ret] = 0;
        return buf;
    }

    virtual void Info(const char * fmt, ...)
    {
		this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[INFO] %s\n", msg);
		this->_lock.Unlock();
    }

    virtual void Warn(const char * fmt, ...)
    {
		this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[WARN] %s\n", msg);
		this->_lock.Unlock();
    }

    virtual void Error(const char * fmt, ...)
    {
		this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[ERROR] %s\n", msg);
		this->_lock.Unlock();
    }

    virtual void Debug(const char * fmt, ...)
    {
		this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[DEBUG] %s\n", msg);
		this->_lock.Unlock();
    }
};

class MockTcpConnection : public TcpConnection
{
public:
    int on_closed_call_count;
    int on_tick_call_count;
    int on_connected_call_count;
    int on_connect_failed_call_count;
    int on_disconnect_call_count;
    int on_disconnected_call_count;
    int on_recv_call_count;

    explicit MockTcpConnection(TcpServer & server) : TcpConnection(server)
    {
        on_closed_call_count = 0;
        on_tick_call_count = 0;
        on_connected_call_count = 0;
        on_connect_failed_call_count = 0;
        on_disconnect_call_count = 0;
        on_disconnected_call_count = 0;
        on_recv_call_count = 0;
    }
    virtual ~MockTcpConnection() {}
    //已关闭连接函数，子类继承
    virtual void OnClosed()
    {
        on_closed_call_count++;
        TcpConnection::OnClosed();
    }
    //定时器处理函数，子类继承
    virtual void OnTick()
    {
        on_tick_call_count++;
    }
    //连接成功通知函数，子类继承
    virtual void OnConnected()
    {
        on_connected_call_count++;
    }
    //连接失败通知函数，子类继承
    virtual void OnConnectFailed()
    {
        on_connect_failed_call_count++;
    }
    //断开连接通知函数，子类继承
    virtual void OnDisconnect()
    {
        on_disconnect_call_count++;
    }
    //已断开连接通知函数，子类继承
    virtual void OnDisconnected()
    {
        on_disconnected_call_count++;
    }
    //接收到数据，通知处理函数，子类继承
    virtual void OnRecv(const char * data, int nread)
    {
        on_recv_call_count++;
    }
};

class MockTcpServer : public TcpServer
{
public:
    int on_closed_call_count;
    int on_tick_call_count;
    int on_connected_call_count;
    int on_connect_failed_call_count;
    int on_disconnect_call_count;
    int on_disconnected_call_count;
    int on_recv_call_count;

    MockTcpServer(ILog * logger = NULL) : TcpServer("MockTcpServer", 1000, 2 * 1024 * 1024, 2 * 1024 * 1024, logger)
    {
        on_closed_call_count = 0;
        on_tick_call_count = 0;
        on_connected_call_count = 0;
        on_connect_failed_call_count = 0;
        on_disconnect_call_count = 0;
        on_disconnected_call_count = 0;
        on_recv_call_count = 0;
    }
    virtual ~MockTcpServer() {}

    //创建连接函数，子类继承
    virtual TcpConnection * NewConnection(TcpServer & server)
    {
        return new MockTcpConnection(server);
    }
    //销毁连接函数，子类继承
    virtual void DestroyConnection(TcpConnection * connection)
    {
        delete connection;
    }
    //已关闭连接函数，子类继承
    virtual void OnClosed()
    {
        on_closed_call_count++;
        TcpServer::OnClosed();
    }
    //定时器处理函数，子类继承
    virtual void OnTick()
    {
        on_tick_call_count++;
    }
    //连接成功通知函数，子类继承
    virtual void OnConnected()
    {
        on_connected_call_count++;
    }
    //连接失败通知函数，子类继承
    virtual void OnConnectFailed()
    {
        on_connect_failed_call_count++;
    }
    //断开连接通知函数，子类继承
    virtual void OnDisconnect()
    {
        on_disconnect_call_count++;
    }
    //已断开连接通知函数，子类继承
    virtual void OnDisconnected()
    {
        on_disconnected_call_count++;
    }
    //接收到数据，通知处理函数，子类继承
    virtual void OnRecv(const char * data, int nread)
    {
        on_recv_call_count++;
    }
};

class MockTcpClient : public TcpClient
{
public:
    int on_closed_call_count;
    int on_tick_call_count;
    int on_connected_call_count;
    int on_connect_failed_call_count;
    int on_disconnect_call_count;
    int on_disconnected_call_count;
    int on_recv_call_count;

    MockTcpClient(ILog * logger = NULL) : TcpClient("MockTcpClient", 1000, 1024 * 1024, 1024 * 1024, logger)
    {
        on_closed_call_count = 0;
        on_tick_call_count = 0;
        on_connected_call_count = 0;
        on_connect_failed_call_count = 0;
        on_disconnect_call_count = 0;
        on_disconnected_call_count = 0;
        on_recv_call_count = 0;
    }
    virtual ~MockTcpClient() {}
    
    //已关闭连接函数，子类继承
    virtual void OnClosed()
    {
        on_closed_call_count++;
        TcpClient::OnClosed();
    }
    //定时器处理函数，子类继承
    virtual void OnTick()
    {
        on_tick_call_count++;
    }
    //连接成功通知函数，子类继承
    virtual void OnConnected()
    {
        on_connected_call_count++;
    }
    //连接失败通知函数，子类继承
    virtual void OnConnectFailed()
    {
        on_connect_failed_call_count++;
    }
    //断开连接通知函数，子类继承
    virtual void OnDisconnect()
    {
        on_disconnect_call_count++;
    }
    //已断开连接通知函数，子类继承
    virtual void OnDisconnected()
    {
        on_disconnected_call_count++;
    }
    //接收到数据，通知处理函数，子类继承
    virtual void OnRecv(const char * data, int nread)
    {
        on_recv_call_count++;
    }
};

/////////////////////////////////////////////////////////////////////////////
// 只继承必须的
/////////////////////////////////////////////////////////////////////////////
class MockNeedTcpConnection : public TcpConnection
{
public:
	explicit MockNeedTcpConnection(TcpServer & server) : TcpConnection(server) {}
	virtual ~MockNeedTcpConnection() {}
};

class MockNeedTcpServer : public TcpServer
{
public:
	MockNeedTcpServer(ILog * logger = NULL) : TcpServer("MockTcpServer", 1000, 2 * 1024 * 1024, 2 * 1024 * 1024, logger) {}
	virtual ~MockNeedTcpServer() {}

	//创建连接函数，子类继承
	virtual TcpConnection * NewConnection(TcpServer & server)
	{
		return new MockNeedTcpConnection(server);
	}
	//销毁连接函数，子类继承
	virtual void DestroyConnection(TcpConnection * connection)
	{
		delete connection;
	}
};

class MockNeedTcpClient : public TcpClient
{
public:
	MockNeedTcpClient(ILog * logger = NULL) : TcpClient("MockTcpClient", 1000, 1024 * 1024, 1024 * 1024, logger) {}
	virtual ~MockNeedTcpClient() {}
};

#endif