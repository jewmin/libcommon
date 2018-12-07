#include "send_packet_pool.h"

SendPacketPool::SendPacketPool(TcpSocket * socket)
    : socket_(socket), send_idx_(0), packet_blocked_(false), last_send_error_(0), current_out_buffer_size_(0), blocked_(false) {
    
}

SendPacketPool::~SendPacketPool() {
    FreeAllPackets();
}

void SendPacketPool::FreeAllPackets() {
    ClearSendQueue();
    Mutex::Guard guard(allocator_lock_);
    allocator_.Clear();
}

void SendPacketPool::ClearSendQueue() {
    send_queue_.Flush();
    Mutex::Guard guard(allocator_lock_);
    allocator_.ReleaseList(static_cast<Packet * *>(send_queue_), send_queue_.Count());
    send_queue_.Clear();
    send_idx_ = 0;
    current_out_buffer_size_ = 0;
}

void SendPacketPool::SendToSocket() {
    bool expected = false;
    if (!blocked_.compare_exchange_strong(expected, true)) {
        return;
    }

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

    if (0 == send_queue_.Count()) {
        send_queue_.Flush();
    }

    if (send_queue_.Count() > 0) {
        socket_->event_loop()->RunInLoop(std::bind(&SendPacketPool::Write, this, send_queue_[send_idx_]));
    } else {
        UnBlocked();
    }
}

void SendPacketPool::Write(Packet * packet) {
    socket_->SendInLoop(reinterpret_cast<const char *>(packet->GetOffsetPtr()), packet->GetReadableLength(), false, std::bind(&SendPacketPool::OnWriteComplete, this, std::placeholders::_1));
}

void SendPacketPool::OnWriteComplete(int status) {
    if (status < 0) {
        last_send_error_ = status;
        if (UV_ECONNRESET == status || UV_ENOTCONN == status || UV_ESHUTDOWN == status || UV_ECONNABORTED == status || UV_EPIPE == status || UV_EBADF == status) {
            socket_->Shutdown();
        }
    } else {
        current_out_buffer_size_ -= static_cast<int>(send_queue_[send_idx_]->GetLength());

        ++send_idx_;
        if (send_idx_ >= send_queue_.Count()) {
            send_idx_ = 0;
            Mutex::Guard guard(allocator_lock_);
            allocator_.ReleaseList(static_cast<Packet * *>(send_queue_), send_queue_.Count());
            send_queue_.Clear();
        }
    }
    UnBlocked();
    SendToSocket();
}