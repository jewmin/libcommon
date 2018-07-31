#include "echo_server.h"
#include "test_protocol.h"
#include "exception.h"
#include "test_log.h"

EchoServer::EchoServer(const std::string & welcome_message, size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size, ILog * logger)
    : SocketServer(max_free_sockets, max_free_buffers, buffer_size, logger), _welcome_message(welcome_message)
{

}

EchoServer::~EchoServer()
{
    try
    {
        this->ReleaseSockets();
        this->ReleaseBuffers();
    }
    catch (...)
    {

    }
}

void EchoServer::OnStartAcceptingConnections()
{
    if (this->_logger)
        this->_logger->Info("OnStartAcceptingConnections");
}

void EchoServer::OnStopAcceptingConnections()
{
    if (this->_logger)
        this->_logger->Info("OnStopAcceptingConnections");
}

void EchoServer::OnShutdownInitiated()
{
    if (this->_logger)
        this->_logger->Info("OnShutdownInitiated");
}

void EchoServer::OnShutdownComplete()
{
    if (this->_logger)
        this->_logger->Info("OnShutdownComplete");
}

void EchoServer::OnConnectionCreated()
{
    if (this->_logger)
        this->_logger->Info("OnConnectionCreated");
}

void EchoServer::OnConnectionEstablished(SocketServer::Socket * socket, Buffer * address)
{
    if (this->_logger)
        this->_logger->Info("OnConnectionEstablished");

    socket->Write(this->_welcome_message.c_str(), this->_welcome_message.length());
}

void EchoServer::OnConnectionClosed(SocketServer::Socket * socket)
{
    if (this->_logger)
        this->_logger->Info("OnConnectionClosed");
}

void EchoServer::OnConnectionDestroyed()
{
    if (this->_logger)
        this->_logger->Info("OnConnectionDestroyed");
}

void EchoServer::PreWrite(SocketServer::Socket * socket, Buffer * buffer, const char * data, size_t data_length)
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

void EchoServer::ReadCompleted(SocketServer::Socket * socket, Buffer * buffer)
{
    try
    {
        buffer = ProcessDataStream(socket, buffer);
        
        socket->Read(buffer);
    }
    catch (const BaseException & ex)
    {
        if (this->_logger)
            this->_logger->Error("ReadCompleted - Exception: %s - %s", ex.Where().c_str(), ex.Message().c_str());

        socket->Shutdown();
    }
    catch (...)
    {
        if (this->_logger)
            this->_logger->Error("ReadCompleted - Unexpected exception");

        socket->Shutdown();
    }
}

void EchoServer::WriteCompleted(SocketServer::Socket * socket, Buffer * buffer)
{
    if (this->_logger)
        this->_logger->Info("WriteCompleted");
}

size_t EchoServer::GetMinimumMessageSize() const
{
    return PACK_HEADER_LEN;
}

size_t EchoServer::GetMessageSize(const Buffer * buffer) const
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

Buffer * EchoServer::ProcessDataStream(SocketServer::Socket * socket, Buffer * buffer) const
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

                    socket->Close();

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

                this->ProcessCommand(socket, buffer);

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

                this->ProcessCommand(socket, message);

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

void EchoServer::ProcessCommand(SocketServer::Socket * socket, Buffer * buffer) const
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
    }
    else
    {
        if (this->_logger)
            this->_logger->Error("found error and close this socket!");

        socket->Close();
    }
}