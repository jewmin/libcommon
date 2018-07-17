#ifndef __LIB_COMMON_TCP_CLIENT_H__
#define __LIB_COMMON_TCP_CLIENT_H__

#include "tcp_service.h"

class TcpClient : public TcpService
{
    friend class TcpService;
    typedef TcpService Super;

public:
    TcpClient(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpClient();

    void Connect(const char * host, uint16_t port);

protected:
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
    uv_connect_t _tcp_connect_req;

    char _name[64];
    uint32_t _tick;
    uint32_t _max_out_buffer_size;
    uint32_t _max_in_buffer_size;

    char _host[256];
    uint16_t _port;
};

#endif