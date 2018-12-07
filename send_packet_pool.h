#ifndef __LIBCOMMON_SEND_PACKET_POOL_H__
#define __LIBCOMMON_SEND_PACKET_POOL_H__

#include <atomic>
#include <functional>

#include "uv.h"
#include "mutex.h"
#include "queue.hpp"
#include "tcp_socket.h"
#include "packet_pool.hpp"
#include "non_copy_able.hpp"

class SendPacketPool : public NonCopyAble {
public:
    SendPacketPool(TcpSocket * socket);
    ~SendPacketPool();

    inline Packet & AllocSendPacket() {
        Mutex::Guard guard(allocator_lock_);
        Packet * packet = allocator_.Allocate();
        packet->SetLength(0);
        return *packet;
    }

    void Flush(Packet & packet) {
        // 如果数据包中被写入了数据则提交到发送队列，否则还原数据包到空闲队列
        if (packet.GetLength() > 0 && current_out_buffer_size_ + packet.GetLength() < socket_->GetMaxOutBufferSize()) {
            // 统计缓冲区大小
            current_out_buffer_size_ += static_cast<int>(packet.GetLength());
            // 调整数据包偏移为0，才能在发送数据的从数据包头部开始发送
            packet.SetPosition(0);
            // 将数据包追加到发送队列中
            send_queue_.Push(&packet);
            // 尝试发送
            if (IsWritable()) {
                SendToSocket();
            }
        } else {
            Mutex::Guard guard(allocator_lock_);
            allocator_.Release(&packet);
        }
    }

    inline void GC() {
        Mutex::Guard guard(allocator_lock_);
        allocator_.GC();
    }

    inline int GetPacketCount() {
        return send_queue_.Count() + send_queue_.AppendCount();
    }

    inline bool IsPacketBlocked() {
        return packet_blocked_;
    }

    inline bool IsWritable() {
        return !blocked_ && SocketOpt::S_CONNECTED == socket_->status();
    }

    inline void UnBlocked() {
        blocked_ = false;
    }

    void OnConnected() {
        UnBlocked();
        SendToSocket();
    }

    void FreeAllPackets();
    void ClearSendQueue();
    void SendToSocket();

protected:
    void Write(Packet * packet);
    void OnWriteComplete(int status);

private:
    TcpSocket * socket_;
    int send_idx_;
    bool packet_blocked_;
    int last_send_error_;
    int current_out_buffer_size_;
    std::atomic<bool> blocked_;
    LockQueue<Packet *, 512> send_queue_;
    Mutex allocator_lock_;
    PacketPool allocator_;
};

#endif