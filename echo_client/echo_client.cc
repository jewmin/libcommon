#include "echo_client.h"
#include "exception.h"
#include "test_protocol.h"
#include "test_log.h"

EchoClient::EchoClient(size_t max_free_buffers, size_t buffer_size, ILog * logger)
    : SocketClient(max_free_buffers, buffer_size, logger)
{

}

EchoClient::~EchoClient()
{
    try
    {
        this->ReleaseBuffers();
    }
    catch (...)
    {

    }
}

void EchoClient::OnStartConnections()
{
    if (this->_logger)
        this->_logger->Info("OnStartConnections");
}

void EchoClient::OnStopConnections()
{
    if (this->_logger)
        this->_logger->Info("OnStopConnections");
}

void EchoClient::OnShutdownInitiated()
{
    if (this->_logger)
        this->_logger->Info("OnShutdownInitiated");
}

void EchoClient::OnShutdownComplete()
{
    if (this->_logger)
        this->_logger->Info("OnShutdownComplete");
}

void EchoClient::OnConnect()
{
    if (this->_logger)
        this->_logger->Info("OnConnect");

    static char message[1000] = { 0 };
    memset(message, '.', 1000);
    memcpy(message, "BEGIN", strlen("BEGIN"));
    memcpy(message + 1000 - strlen("END"), "END", 3);
    this->Write(message, 1000);
}

void EchoClient::OnConnectFail()
{
    if (this->_logger)
        this->_logger->Info("OnConnectFail");
}

void EchoClient::OnClose()
{
    if (this->_logger)
        this->_logger->Info("OnClose");
}

void EchoClient::ReadCompleted(Buffer * buffer)
{
    try
    {
        buffer = ProcessDataStream(buffer);
        
        this->Read(buffer);
    }
    catch (const BaseException & ex)
    {
        if (this->_logger)
            this->_logger->Error("EchoClient::ReadCompleted() - Exception: %s - %s", ex.Where().c_str(), ex.Message().c_str());

        this->StopConnections();
    }
    catch (...)
    {
        if (this->_logger)
            this->_logger->Error("EchoClient::ReadCompleted() - Unexpected exception");

        this->StopConnections();
    }
}

void EchoClient::WriteCompleted(Buffer * buffer, int status)
{
    if (this->_logger)
        this->_logger->Info("WriteCompleted %s", uv_strerror(status));

    /*static char message[1000] = { 0 };
    memset(message, '.', 1000);
    memcpy(message, "BEGIN", strlen("BEGIN"));
    memcpy(message + 1000 - strlen("END"), "END", 3);
    this->Write(message, 1000);*/
}

void EchoClient::PreWrite(Buffer * buffer, const char * data, size_t data_length)
{
    if (buffer && data_length > 0)
    {
        PACK_HEADER ph = {0};

        ph.pack_begin_flag = PACK_BEGIN_FLAG;
        ph.pack_end_flag = PACK_END_FLAG;

        ph.data_len = (uint16_t)data_length;
        ph.crc_data = MAKE_CRC_DATE(PACK_BEGIN_FLAG, PACK_END_FLAG, (uint16_t)data_length);

        buffer->AddData((const char *)&ph, PACK_HEADER_LEN);
    }
}

size_t EchoClient::GetMinimumMessageSize() const
{
    return PACK_HEADER_LEN;
}

size_t EchoClient::GetMessageSize(const Buffer * buffer) const
{
    const uint8_t * data = buffer->GetBuffer();
    const size_t used = buffer->GetUsed();

    PACK_HEADER ph = {0};

    /*
     * First, verify the flag of a message
     */
    if (used > PACK_HEADER_LEN)
    {
        memcpy(&ph, data, PACK_HEADER_LEN);

        if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag)
        {
            uint16_t crc = MAKE_CRC_DATE(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);

            if (ph.crc_data == crc)
            {
                return ph.data_len + PACK_HEADER_LEN;
            }
        }
    }

    return 0;
}

Buffer * EchoClient::ProcessDataStream(Buffer * buffer)
{
    bool done;

    do
    {
        done = true;
        
        const size_t used = buffer->GetUsed();
        
        if (used >= this->GetMinimumMessageSize())
        {
            const size_t message_size = this->GetMessageSize(buffer);
            
            if (message_size == 0)
            {
                /*
                 * havent got a complete message yet.
                
                 * we null terminate our messages in the buffer, so we need to reserve
                 * a byte of the buffer for this purpose...
                 */
                if (used == (buffer->GetSize() - 1))
                {
                    if (this->_logger)
                        this->_logger->Info("Too much data!");

                    /*
                     * Write this message and then shutdown the sending side of the socket.
                     */
                    if (this->_logger)
                        this->_logger->Error("found error and close this socket!");

                    this->StopConnections();

                    /*
                     * throw the rubbish away
                     */
                    buffer->Empty();

                    done = true;
                }
            }
            else if (used == message_size)
            {
                if (this->_logger)
                    this->_logger->Info("Got complete, distinct, message");

                /*
                 * we have a whole, distinct, message
                 */
                buffer->AddData(0);

                this->ProcessCommand(buffer);

                buffer->Empty();

                done = true;
            }
            else if (used > message_size)
            {
                if (this->_logger)
                    this->_logger->Info("Got message plus extra data");

                /*
                 * we have a message, plus some more data
                 * 
                 * allocate a new buffer, copy the extra data into it and try again...
                 */
                
                Buffer * message = buffer->SplitBuffer(message_size);

                message->AddData(0);

                this->ProcessCommand(message);

                message->Release();

                /*
                 * loop again, we may have another complete message in there...
                 */
                done = false;
            }
        }
    }
    while (!done);

    /*
     * not enough data in the buffer, reissue a read into the same buffer to collect more data
     */
    return buffer;
}

void EchoClient::ProcessCommand(Buffer * buffer)
{
    const uint8_t * pack_data = buffer->GetBuffer();
    const size_t used = buffer->GetUsed();

    bool ok = false;

    uint16_t data_len = 0;
    PACK_HEADER ph = {0};

    if (used > PACK_HEADER_LEN)
    {
        memcpy(&ph, pack_data, PACK_HEADER_LEN);

        data_len = ph.data_len;

        ok = true;
    }

    if (ok)
    {
        const uint8_t * data = pack_data + PACK_HEADER_LEN;
        const size_t data_length = data_len;

        /*
         * TODO : Process data receive from client
         */
        if (this->_logger)
            this->_logger->Info("Package[length:%u]\n%s", data_len, DumpData(data, data_length, 40).c_str());

        this->Write((const char *)data, data_length);
    }
    else
    {
        if (this->_logger)
            this->_logger->Error("found error and close this socket!");

        this->StopConnections();
    }
}