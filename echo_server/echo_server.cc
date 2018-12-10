#include <algorithm>
#include "echo_server.h"
#include "test_protocol.h"
#include "test_log.h"

EchoServer::EchoServer(const std::string & welcome_message, EventLoop * loop)
    : TcpServer(loop, "EchoServer", 1000, 64 * 1024 * 1024, 1024, 1), welcome_message_(welcome_message)
    , run_second_(0), send_total_(0), recv_total_(0), current_second_send_(0), current_second_recv_(0)
    , last_second_send_(0), last_second_recv_(0), high_send_pre_second_(0), high_recv_pre_second_(0)
    , send_average_(0.0), recv_average_(0.0) {
    statistics_timer_ = event_loop()->RunEvery(1000, std::bind(&EchoServer::OnStatisticsTick, this));
}

EchoServer::~EchoServer() {
    
}

void EchoServer::OnTickEvent(TcpConnection * conn) {
    SendMessage(conn);
}

void EchoServer::OnShutdownInitiated() {
    if (statistics_timer_ > 0) {
        event_loop()->Cancel(statistics_timer_);
        statistics_timer_ = 0;
    }
}

void EchoServer::OnReadCompleted(TcpConnection * conn, Packet * packet) {
    current_second_recv_ += static_cast<uint32_t>(packet->GetLength());
    packet->SetPosition(0);

    bool done;
    do {
        done = true;
        const size_t used = packet->GetReadableLength();
        if (used >= GetMinimumMessageSize()) { // header
            const size_t message_size = GetMessageSize(packet); // header + body
            if (message_size == 0) {
                packet->AdjustOffset(1);
                done = false;
            } else if (used == message_size) {
                ProcessCommand(packet);
                done = true;
            } else if (used > message_size) {
                ProcessCommand(packet);
                done = false;
            }
        }
    } while (!done);

    if (packet->GetReadableLength() > 0) {
        memmove(packet->GetMemoryPtr(), packet->GetOffsetPtr(), packet->GetReadableLength());
        packet->SetLength(packet->GetReadableLength());
    }
    else {
        packet->SetLength(0);
    }
}

void EchoServer::OnStatisticsTick() {
    ++run_second_;
    send_total_ += current_second_send_;
    recv_total_ += current_second_recv_;

    high_send_pre_second_ = std::max<uint32_t>(high_send_pre_second_, current_second_send_);
    high_recv_pre_second_ = std::max<uint32_t>(high_recv_pre_second_, current_second_recv_);

    send_average_ = static_cast<double>(send_total_) / run_second_;
    recv_average_ = static_cast<double>(recv_total_) / run_second_;

    last_second_send_ = current_second_send_;
    last_second_recv_ = current_second_recv_;

    current_second_send_ = 0;
    current_second_recv_ = 0;

    if (log()) {
        log()->LogInfo("下行:%.2fK/%.2fK/%.2fK ; 上行:%.2fK/%.2fK/%.2fK(即/峰/总)", last_second_recv_ / 1000.0, high_recv_pre_second_ / 1000.0, recv_total_ / 1000.0,
            last_second_send_ / 1000.0, high_send_pre_second_ / 1000.0, send_total_ / 1000.0);
    }
}

void EchoServer::SendMessage(TcpConnection * conn) {
    SendMessage(conn, welcome_message_.c_str(), welcome_message_.size());
}

void EchoServer::SendMessage(TcpConnection * conn, const char * message, size_t length) {
    if (message && length > 0) {
        PACK_HEADER ph = { 0 };

        ph.pack_begin_flag = PACK_BEGIN_FLAG;
        ph.pack_end_flag = PACK_END_FLAG;
        ph.data_len = static_cast<uint16_t>(length);
        ph.crc_data = MAKE_CRC_DATE(PACK_BEGIN_FLAG, PACK_END_FLAG, static_cast<uint16_t>(length));

        Packet & packet = conn->AllocSendPacket();
        packet.WriteBinary(reinterpret_cast<const uint8_t *>(&ph), PACK_HEADER_LEN);
        packet.WriteBinary(reinterpret_cast<const uint8_t *>(message), length);
        current_second_send_ += static_cast<uint32_t>(packet.GetLength());
        conn->Flush(packet);
    }
}

size_t EchoServer::GetMinimumMessageSize() const {
    return PACK_HEADER_LEN;
}

size_t EchoServer::GetMessageSize(Packet * packet) const {
    const uint8_t * data = packet->GetOffsetPtr();

    PACK_HEADER ph = { 0 };
    memcpy(&ph, data, PACK_HEADER_LEN);
    if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag) {
        uint16_t crc = MAKE_CRC_DATE(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
        if (ph.crc_data == crc) {
            return ph.data_len + PACK_HEADER_LEN;
        }
    }

    return 0;
}

void EchoServer::ProcessCommand(Packet * packet) {
    const uint8_t * pack_data = packet->GetOffsetPtr();
    const uint8_t * data = pack_data + PACK_HEADER_LEN;

    PACK_HEADER ph = { 0 };
    memcpy(&ph, pack_data, PACK_HEADER_LEN);
    const size_t data_length = ph.data_len;
    if (log()) {
        log()->LogDebug("Package[length:%zu]\n%s", data_length, DumpData(data, data_length, 49).c_str());
    }

    packet->AdjustOffset(PACK_HEADER_LEN + ph.data_len);
}
