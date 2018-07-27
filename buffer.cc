#include "buffer.h"
#include "exception.h"

Buffer::Buffer(Allocator & allocator, size_t size)
    : _allocator(allocator), _ref(1), _size(size), _used(0)
{
    this->Empty();
}

uv_buf_t * Buffer::GetUVBuffer() const
{
    return (uv_buf_t *)&this->_uv_buf;
}

const uint8_t * Buffer::GetBuffer() const
{
    return &this->_buffer[0];
}

size_t Buffer::GetUsed() const
{
    return this->_used;
}

size_t Buffer::GetSize() const
{
    return this->_size;
}

void Buffer::SetupRead()
{
    if (this->_used == 0)
    {
        this->_uv_buf.base = (char *)this->_buffer;
        this->_uv_buf.len = (ULONG)this->_size;
    }
    else
    {
        this->_uv_buf.base = (char *)(this->_buffer + this->_used);
        this->_uv_buf.len = (ULONG)(this->_size - this->_used);
    }
}

void Buffer::SetupWrite()
{
    this->_uv_buf.base = (char *)this->_buffer;
    this->_uv_buf.len = (ULONG)this->_used;

    this->_used = 0;
}

void Buffer::AddData(const char * const data, size_t data_length)
{
    if (data_length > this->_size - this->_used)
        throw BaseException("Buffer::AddData()", "Not enough space in buffer");

    memcpy(this->_buffer + this->_used, data, data_length);

    this->_used += data_length;
}

void Buffer::AddData(const uint8_t * const data, size_t data_length)
{
    this->AddData((const char *)data, data_length);
}

void Buffer::AddData(char data)
{
    this->AddData(&data, 1);
}

void Buffer::Use(size_t data_used)
{
    this->_used += data_used;
}

Buffer * Buffer::SplitBuffer(size_t bytes_to_remove)
{
    Buffer * buffer = this->_allocator.Allocate();

    buffer->AddData(this->_buffer, bytes_to_remove);
    
    this->_used -= bytes_to_remove;

    memmove(this->_buffer, this->_buffer + bytes_to_remove, this->_used);

    return buffer;
}

void Buffer::Empty()
{
    this->_uv_buf.base = (char *)this->_buffer;
    this->_uv_buf.len = (ULONG)this->_size;

    this->_used = 0;
}

void Buffer::AddRef()
{
    this->_lock.Lock();
    ++this->_ref;
    this->_lock.Unlock();
}

void Buffer::Release()
{
    if (this->_ref == 0)
        throw BaseException("Buffer::Release()", "_ref is already zero");

    this->_lock.Lock();
    int ref = --this->_ref;
    this->_lock.Unlock();

    if (ref == 0)
        this->_allocator.Release(this);
}

void * Buffer::operator new(size_t object_size, size_t buffer_size)
{
    void * mem = jc_malloc(object_size + buffer_size);
    return mem;
}

void Buffer::operator delete(void * object)
{
    jc_free(object);
}

Buffer::Allocator::Allocator(size_t buffer_size, size_t max_free_buffers)
    : _buffer_size(buffer_size), _max_free_buffers(max_free_buffers)
{

}

Buffer::Allocator::~Allocator()
{
    this->Flush();
}

Buffer * Buffer::Allocator::Allocate()
{
    Mutex::Owner lock(this->_lock);

    Buffer * buffer = NULL;

    if (!this->_free_list.empty())
    {
        buffer = this->_free_list.front();
        this->_free_list.pop_front();
        buffer->AddRef();
    }
    else
    {
        buffer = new(this->_buffer_size)Buffer(*this, this->_buffer_size);
        if (!buffer)
            throw BaseException("Buffer::Allocator::Allocate()", "Out of memory");
        
        /*
         * Call to unqualified virtual function
         */
        this->OnBufferCreated();
    }

    this->_active_list.push_back(buffer);

    /*
     * call to unqualified virtual function
     */
    this->OnBufferAllocated();

    return buffer;
}

void Buffer::Allocator::Flush()
{
    Mutex::Owner lock(this->_lock);

    while (!this->_active_list.empty())
    {
        /*
         * Call to unqualified virtual function
         */
        this->OnBufferReleased();

        this->DestroyBuffer(this->_active_list.front());
        this->_active_list.pop_front();
    }

    while (!this->_free_list.empty())
    {
        this->DestroyBuffer(this->_free_list.front());
        this->_free_list.pop_front();
    }
}

void Buffer::Allocator::Release(Buffer * buffer)
{
    if (!buffer)
        throw BaseException("Buffer::Allocator::Release()", "buffer is null");

    Mutex::Owner lock(this->_lock);
    
    /*
     * Call to unqualified virtual function
     */
    this->OnBufferReleased();

    /*
     * unlink from the in use list
     */
    this->_active_list.remove(buffer);

    if (this->_max_free_buffers == 0 || this->_free_list.size() < this->_max_free_buffers)
    {
        buffer->Empty();

        /*
         * Add to the free list
         */
        this->_free_list.push_back(buffer);
    }
    else
    {
        this->DestroyBuffer(buffer);
    }
}

void Buffer::Allocator::DestroyBuffer(Buffer * buffer)
{
    delete buffer;

    /*
     * Call to unqualified virtual function
     */
    this->OnBufferDestroyed();
}