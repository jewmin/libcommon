#include "send_packet_pool.h"

SendPacketPool::SendPacketPool(TcpSocket * socket)
    : socket_(socket), packet_blocked_(false), last_send_error_(0) {
    
}

SendPacketPool::~SendPacketPool() {
    FreeAllPackets();
}

void SendPacketPool::FreeAllPackets() {
    ClearSendList();
    Mutex::Guard guard(allocator_lock_);
    allocator_.ReleaseList(static_cast<Packet * *>(gc_list_), gc_list_.Count());
    gc_list_.Clear();
    allocator_.Clear();
}

void SendPacketPool::ClearSendList() {
    Mutex::Guard guard(send_lock_);
    gc_list_.Reserve(gc_list_.Count() + GetPacketCount());
    
    for (auto & it : ready_list_) {
        gc_list_.Add(it);
    }
    ready_list_.clear();

    for (auto & it : send_list_) {
        gc_list_.Add(it);
    }
    send_list_.clear();
}

void SendPacketPool::SendToSocket() {
    if (socket_->IsWriting()) {
        return;
    }

    socket_->EnableWriting();

    if (GetPacketCount() >= 1024) {
        packet_blocked_ = true;
        static uint64_t next_t = 0;
        uint64_t now_t = uv_now(socket_->uv_loop());
        if (next_t < now_t) {
            if (socket_->log()) {
                socket_->log()->LogError("too many send packets (%d) in SendPacketPool! last error code (%d).", GetPacketCount(), last_send_error_);
            }
            next_t = now_t + 10; // 10ºÁÃëÊä³öÒ»´Î
        }
    } else {
        packet_blocked_ = false;
    }

    SwapSendList();

    if (send_list_.size() > 0) {
        Packet * packet = send_list_.front();
        socket_->WriteInLoop(reinterpret_cast<const char *>(packet->GetOffsetPtr()), packet->GetReadableLength(), false, std::bind(&SendPacketPool::OnWriteComplete, this, std::placeholders::_1));
    } else {
        socket_->DisableWriting();
    }
}

void SendPacketPool::OnWriteComplete(int status) {
    if (status < 0) {
        last_send_error_ = status;
        socket_->ShutdownInLoop();
    } else {
        Packet * packet = send_list_.front();
        send_list_.pop_front();
        gc_list_.Add(packet);
        socket_->DisableWriting();
        SendToSocket();
    }
}