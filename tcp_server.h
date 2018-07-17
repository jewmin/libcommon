#ifndef __LIB_COMMON_TCP_SERVER_H__
#define __LIB_COMMON_TCP_SERVER_H__

#include "tcp_service.h"

class TcpConnection;
class TcpServer : public TcpService
{
    friend class TcpService;
    friend class TcpConnection;
    typedef TcpService Super;

public:
    TcpServer(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpServer();

    void Listen(const char * host, uint16_t port);

protected:
    //�������Ӻ���������̳�
    virtual TcpConnection * NewConnection(TcpServer & server) = 0;
    //�������Ӻ���������̳�
    virtual void DestroyConnection(TcpConnection * connection) = 0;
    //�ѹر����Ӻ���������̳�
    virtual void OnClosed();
    //��ʱ��������������̳�
    virtual void OnTick();
    //���ӳɹ�֪ͨ����������̳�
    virtual void OnConnected();
    //����ʧ��֪ͨ����������̳�
    virtual void OnConnectFailed();
    //�Ͽ�����֪ͨ����������̳�
    virtual void OnDisconnect();
    //�ѶϿ�����֪ͨ����������̳�
    virtual void OnDisconnected();
    //���յ����ݣ�֪ͨ������������̳�
    virtual void OnRecv(const char * data, int nread);

protected:
    uv_timer_t _tick_handle;

    char _name[64];
    uint32_t _tick;
    uint32_t _max_out_buffer_size;
    uint32_t _max_in_buffer_size;

    char _host[256];
    uint16_t _port;
};

#endif