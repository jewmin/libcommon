#ifndef __LIB_COMMON_TCP_SERVICE_H__
#define __LIB_COMMON_TCP_SERVICE_H__

#include "service.h"

class TcpService : public BaseService
{
    typedef BaseService Super;

public:
    explicit TcpService(ILog * logger = NULL);
    virtual ~TcpService();

protected:
    //��ʱ��������������̳�
    virtual void OnTick() = 0;
    //���ӳɹ�֪ͨ����������̳�
    virtual void OnConnected() = 0;
    //����ʧ��֪ͨ����������̳�
    virtual void OnConnectFailed() = 0;
    //�Ͽ�����֪ͨ����������̳�
    virtual void OnDisconnect() = 0;
    //�ѶϿ�����֪ͨ����������̳�
    virtual void OnDisconnected() = 0;
    //���յ����ݣ�֪ͨ������������̳�
    virtual void OnRecv(const char * data, int nread) = 0;

    //libuv�ص�������
    static void TimerCallback(uv_timer_t * handle);
    static void ConnectionCallback(uv_stream_t * server, int status);
    static void CloseCallback(uv_handle_t * handle);
    static void ConnectCallback(uv_connect_t * req, int status);
    static void AllocBufferCallback(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);

protected:
    union {
        uv_handle_t handle;
        uv_stream_t stream;
        uv_tcp_t tcp;
    } _handle;
};

#endif