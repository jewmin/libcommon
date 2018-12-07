#ifndef __LIBCOMMON_SEND_PACKET_POOL_H__
#define __LIBCOMMON_SEND_PACKET_POOL_H__

#include <list>
#include <functional>

#include "vector.hpp"
#include "tcp_socket.h"
#include "packet_pool.hpp"
#include "non_copy_able.hpp"

class SendPacketPool : public NonCopyAble {
public:
    SendPacketPool(TcpSocket * socket);
    virtual ~SendPacketPool();

    inline Packet & AllocSendPacket() {
        Mutex::Guard guard(allocator_lock_);
        Packet * packet = allocator_.Allocate();
        packet->SetLength(0);
        return *packet;
    }

    inline void Flush(Packet & packet) {
        if (packet.GetLength() > 0) {
            packet.SetPosition(0);
            Mutex::Guard guard(send_lock_);
            ready_list_.push_back(&packet);
            TryWrite();
        } else {
            Mutex::Guard guard(allocator_lock_);
            allocator_.Release(&packet);
        }
    }

    inline void TryWrite() {
        if (IsWritable()) {
            socket_->event_loop()->QueueInLoop(std::bind(&SendPacketPool::SendToSocket, this));
        }
    }

    inline void SwapSendList() {
        if (send_list_.size() == 0 && ready_list_.size() > 0) {
            Mutex::Guard guard(send_lock_);
            send_list_.swap(ready_list_);
        }
    }

    inline void GC() {
        Mutex::Guard guard(allocator_lock_);
        allocator_.ReleaseList(static_cast<Packet * *>(gc_list_), gc_list_.Count());
        gc_list_.Clear();
    }

    inline int GetPacketCount() {
        return static_cast<int>(ready_list_.size() + send_list_.size());
    }

    inline bool IsPacketBlocked() {
        return packet_blocked_;
    }

    inline bool IsWritable() {
        return SocketOpt::S_CONNECTED == socket_->status() && !socket_->IsWriting();
    }

    void FreeAllPackets();
    void ClearSendList();
    void SendToSocket();

protected:
    void OnWriteComplete(int status);

private:
    TcpSocket * socket_;
    bool packet_blocked_;
    int last_send_error_;
    
    Mutex send_lock_;
    std::list<Packet *> ready_list_;    // 等待发送队列
    std::list<Packet *> send_list_;     // 正在发送队列
    BaseVector<Packet *, 64> gc_list_;  // 回收队列
    
    Mutex allocator_lock_;
    PacketPool allocator_;
};

#endif