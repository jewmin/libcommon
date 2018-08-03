#ifndef __LIB_COMMON_BUFFER_H__
#define __LIB_COMMON_BUFFER_H__

#include "uv.h"
#include <list>
#include "mutex.h"
#include "common.h"

class Buffer
{
public:
    class Allocator;
    friend class Allocator;

    uv_buf_t * GetUVBuffer() const;
    const uint8_t * GetBuffer() const;
    size_t GetUsed() const;
    size_t GetSize() const;

    void SetupRead();
    void SetupWrite(size_t offset = 0);

    void AddData(const char * const data, size_t data_length);
    void AddData(const uint8_t * const data, size_t data_length);
    void AddData(char data);
    void Use(size_t data_used);

    Buffer * SplitBuffer(size_t bytes_to_remove);
    void Empty();

    void AddRef();
    void Release();

private:
    static void * operator new(size_t object_size, size_t buffer_size);
    static void operator delete(void * object);

    Buffer(Allocator & allocator, size_t size);

    /*
     * No copies do not implement
     */
    Buffer(const Buffer & rhs);
    Buffer & operator =(const Buffer & rhs);

private:
    uv_buf_t _uv_buf;
    Allocator & _allocator;
    Mutex _lock;

    long _ref;
    const size_t _size;
    size_t _used;

    /*
     * Start of the actual buffer, must remain the last
     */
    uint8_t _buffer[1];
};

class Buffer::Allocator
{
    friend class Buffer;
    typedef std::list<Buffer *> BufferList;

public:
    explicit Allocator(size_t buffer_size, size_t max_free_buffers);
    virtual ~Allocator();

    Buffer * Allocate();

protected:
    void Flush();

private:
    void Release(Buffer * buffer);
    void DestroyBuffer(Buffer * buffer);

    virtual void OnBufferCreated() {}
    virtual void OnBufferAllocated() {}
    virtual void OnBufferReleased() {}
    virtual void OnBufferDestroyed() {}

    /*
     * No copies do not implement
     */
    Allocator(const Allocator & rhs);
    Allocator & operator =(const Allocator & rhs);

private:
    const size_t _buffer_size;
    const size_t _max_free_buffers;

    BufferList _free_list;
    BufferList _active_list;

    Mutex _lock;
};

#endif