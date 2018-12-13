#include "echo_client.h"
#include "test_protocol.h"
#include "test_log.h"

EchoClient::EchoClient(EventLoop * loop)
    : TcpClient(loop, "EchoClient", 1000, 5000, 64 * 1024 * 1024, 1024) {

}

EchoClient::~EchoClient() {
    
}

void EchoClient::OnTickEvent() {
    SendMessage();
}

void EchoClient::OnReadCompleted(Packet * packet) {
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
    } else {
        packet->SetLength(0);
    }
}

void EchoClient::SendMessage() {
    static char message[1000] = { 0 };
    memset(message, '.', sizeof(message));
    memcpy(message, "BEGIN", strlen("BEGIN"));
    memcpy(message + 1000 - strlen("END"), "END", strlen("END"));
    SendMessage(message, sizeof(message));
}

void EchoClient::SendMessage(const char * message, size_t length) {
    if (message && length > 0) {
        PACK_HEADER ph = { 0 };

        ph.pack_begin_flag = PACK_BEGIN_FLAG;
        ph.pack_end_flag = PACK_END_FLAG;
        ph.data_len = static_cast<uint16_t>(length);
        ph.crc_data = MAKE_CRC_DATE(PACK_BEGIN_FLAG, PACK_END_FLAG, static_cast<uint16_t>(length));

        Packet & packet = AllocSendPacket();
        packet.WriteBinary(reinterpret_cast<const uint8_t *>(&ph), PACK_HEADER_LEN);
        packet.WriteBinary(reinterpret_cast<const uint8_t *>(message), length);
        Flush(packet);
    }
}

size_t EchoClient::GetMinimumMessageSize() const {
    return PACK_HEADER_LEN;
}

size_t EchoClient::GetMessageSize(Packet * packet) const {
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

void EchoClient::ProcessCommand(Packet * packet) {
    const uint8_t * pack_data = packet->GetOffsetPtr();
    const uint8_t * data = pack_data + PACK_HEADER_LEN;

    PACK_HEADER ph = { 0 };
    memcpy(&ph, pack_data, PACK_HEADER_LEN);
    const size_t data_length = ph.data_len;
    if (log()) {
        log()->LogTrace("Package[length:%zu]\n%s", data_length, DumpData(data, data_length, 49).c_str());
    }

    packet->AdjustOffset(PACK_HEADER_LEN + ph.data_len);
}