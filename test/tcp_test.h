#ifndef __UNIT_TEST_TCP_TEST_H__
#define __UNIT_TEST_TCP_TEST_H__

#include "tcp_client.h"
#include "tcp_server.h"
#include "queue.hpp"
#include "packet.hpp"
#include "object_pool.hpp"
#include "def.h"

class MockTcpClient : protected TcpClient {
public:
    MockTcpClient(const char * name, size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = nullptr)
        : TcpClient(name, max_free_buffers, buffer_size, logger)
        , buffer_created_count_(0), buffer_alloced_count_(0), send_idx_(0) {

    }

    virtual ~MockTcpClient() {
        FreeAllPackets();
    }

    inline Packet & AllocSendPack() {
        Mutex::Guard guard(allocator_lock_);
        Packet * packet = allocator_.Allocate();
        packet->SetLength(0);
        return *packet;
    }

    inline void Flush(Packet & pakcet) {
        // 如果数据包中被写入了数据则提交到发送队列，否则还原数据包到空闲队列
        if (pakcet.GetLength() > 0) {
            // 调整数据包偏移为0，才能在发送数据的从数据包头部开始发送
            pakcet.SetPosition(0);
            // 将数据包追加到发送队列中
            send_queue_.Push(&pakcet);
            // 尝试发送
        } else {
            Mutex::Guard guard(allocator_lock_);
            allocator_.Release(&pakcet);
        }
    }

    void FreeAllPackets() {
        send_queue_.Flush();
        Mutex::Guard guard(allocator_lock_);
        allocator_.ReleaseList(static_cast<Packet * *>(send_queue_), send_queue_.Count());
        send_queue_.Clear();
        send_idx_ = 0;
    }

protected:
    void SendToSocket() {
        if (send_queue_.Count() == 0) {
            send_queue_.Flush();
        }

        if (!HasFlag(SocketOpt::F_WRITING) && send_queue_.Count() > 0) {
            AddFlag(SocketOpt::F_WRITING);
            Packet * packet = send_queue_[send_idx_];
            SendInLoop(reinterpret_cast<const char *>(packet->GetOffsetPtr()), packet->GetAvaliableLength(), std::bind(&MockTcpClient::OnWriteComplete, this, std::placeholders::_1));
        }
    }

    void SendRegisteClient() {
        server_regdata_t reg_data;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.game_type = server_regdata_t::GT_ID;
        reg_data.server_type = GetLocalServerType();
        reg_data.server_index = GetLocalServerIndex();
        STRNCPY_S(reg_data.server_name, GetLocalServerName());
        SendInLoop(reinterpret_cast<const char *>(&reg_data), sizeof(reg_data));
    }

    virtual int GetLocalServerType() {
        return GateServer;
    }

    virtual const char * GetLocalServerName() {
        return name();
    }

    virtual int GetLocalServerIndex() {
        return 1;
    }

    void OnConnected() override {
        if (logger()) {
            logger()->LogInfo("%s server succ", name());
        }
        SendRegisteClient();
    }
    
    void OnConnectFailed() override {
        if (logger()) {
            logger()->LogInfo("%s server fail", name());
        }
        // 重连定时器
    }
    
    void OnDisconnected() override {
        if (logger()) {
            logger()->LogInfo("%s server disconnect", name());
        }
    }

    void OnReadComplete(Buffer * buffer) override {
        
    }

    void OnWriteComplete(int status) {
        if (status > 0) {
            ++send_idx_;
            if (send_idx_ >= send_queue_.Count()) {
                send_idx_ = 0;
                Mutex::Guard guard(allocator_lock_);
                allocator_.ReleaseList(static_cast<Packet * *>(send_queue_), send_queue_.Count());
                send_queue_.Clear();
            }
        }
        RemoveFlag(SocketOpt::F_WRITING);
        SendToSocket();
    }

private:
    void OnBufferCreated() override {
        ++buffer_created_count_;
    }

    void OnBufferAllocated() override {
        ++buffer_alloced_count_;
    }

    void OnBufferReleased() override {
        --buffer_alloced_count_;
    }

    void OnBufferDestroyed() override {
        --buffer_created_count_;
    }

public:
    int buffer_created_count_;
    int buffer_alloced_count_;
    int send_idx_;
    Mutex allocator_lock_;
    ObjectPool<Packet> allocator_;
    LockQueue<Packet *> send_queue_;
};

class MockTcpServer : protected TcpServer {
public:
    MockTcpServer(const char * name, size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = nullptr)
        : TcpServer(name, max_free_sockets, max_free_buffers, buffer_size, logger)
        , buffer_created_count_(0), buffer_alloced_count_(0) {

    }
    virtual ~MockTcpServer() {

    }

protected:
    virtual void OnConnectionCreated() override {

    }
    
    virtual void OnConnectionEstablished(Socket * socket, Buffer * address) override {

    }
    
    virtual void OnConnectionClosed(Socket * socket) override {

    }
    
    virtual void OnConnectionDestroyed() override {

    }
    
    virtual void OnReadComplete(Socket * socket, Buffer * buffer) override {

    }
    
    virtual void OnWriteComplete(Socket * socket, int written) override {
        if (written > 0) {

        }
    }

private:
    virtual void OnBufferCreated() override {
        ++buffer_created_count_;
    }

    virtual void OnBufferAllocated() override {
        ++buffer_alloced_count_;
    }

    virtual void OnBufferReleased() override {
        --buffer_alloced_count_;
    }

    virtual void OnBufferDestroyed() override {
        --buffer_created_count_;
    }

public:
    int buffer_created_count_;
    int buffer_alloced_count_;
};

#endif