#ifndef __LIBCOMMON_TCP_SOCKET_H__
#define __LIBCOMMON_TCP_SOCKET_H__

#include <vector>
#include <functional>

#include "uv.h"
#include "common.h"
#include "packet.hpp"
#include "event_loop.h"
#include "socket_opt.h"
#include "non_copy_able.hpp"

class SendPacketPool;
class TcpSocket : public SocketOpt, public NonCopyAble {
public:
    friend class SendPacketPool;
    using WriteCompleteCallback = std::function<void(int)>;
    class WriteRequest {
    public:
        WriteRequest(TcpSocket * socket, const char * data, size_t size, bool assign, const WriteCompleteCallback & cb)
            : socket_(socket), write_complete_cb_(cb) {
            req_.data = this;
            if (assign) {
                storage_.assign(data, data + size);
                buf_ = uv_buf_init(&storage_[0], static_cast<unsigned int>(storage_.size()));
            } else {
                buf_ = uv_buf_init(const_cast<char *>(data), static_cast<unsigned int>(size));
            }
        }

        TcpSocket * const socket_;
        std::vector<char> storage_;
        uv_write_t req_;
        uv_buf_t buf_;
        const WriteCompleteCallback write_complete_cb_;
    };

    inline uv_tcp_t * uv_tcp() {
        return &tcp_;
    }

    inline uv_stream_t * uv_stream() {
        return reinterpret_cast<uv_stream_t *>(&tcp_);
    }

    inline uv_handle_t * uv_handle() {
        return reinterpret_cast<uv_handle_t *>(&tcp_);
    }

    inline EventLoop * event_loop() const {
        return loop_;
    }

    inline uv_loop_t * uv_loop() const {
        return loop_->uv_loop();
    }

    inline Logger * log() const {
        return loop_->log();
    }

    inline const char * name() const {
        return name_;
    }

    inline void SetHost(const char * host) {
        STRNCPY_S(host_, host);
    }

    inline const char * GetHost() const {
        return host_;
    }

    inline void SetPort(uint16_t port) {
        port_ = port;
    }

    inline uint16_t GetPort() const {
        return port_;
    }

    inline int GetMaxOutBufferSize() const {
        return max_out_buffer_size_;
    }

    inline int GetMaxInBufferSize() const {
        return max_in_buffer_size_;
    }

    void WriteInLoop(const char * data, size_t size, bool assign = true, const WriteCompleteCallback & cb = nullptr);

protected:
    TcpSocket(EventLoop * loop, const char * name, const int max_out_buffer_size, const int max_in_buffer_size);
    virtual ~TcpSocket();

    void ListenInLoop();
    void ConnectInLoop();
    void ShutdownInLoop();
    void CloseInLoop();
    void ReadStartInLoop();
    void ReadStopInLoop();
    int AcceptInLoop(TcpSocket * accept_socket);

    virtual void OnConnected() {}
    virtual void OnConnectFailed() {}
    virtual void OnDisconnected() {}
    //virtual void OnReadCompleted(const char * data, size_t size) {}
    virtual void OnReadCompleted(Packet * packet) {}
    virtual TcpSocket * AllocateSocket() { return nullptr; }

private:
    static void NewConnection(uv_stream_t * stream, int status);
    static void AfterConnect(uv_connect_t * req, int status);
    static void AfterClose(uv_handle_t * handle);
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void AfterWrite(uv_write_t * req, int status);

protected:
    EventLoop * loop_;
    const int max_out_buffer_size_;
    const int max_in_buffer_size_;

private:
    uv_tcp_t tcp_;
    uv_connect_t connect_;
    Packet recv_buffer_;

    char name_[128];
    char host_[128];    // 监听/连接/对端地址
    uint16_t port_;     // 监听/连接/对端端口
};

#endif