#ifndef __LIBCOMMON_BUFFER_H__
#define __LIBCOMMON_BUFFER_H__

#include <atomic>

#include "uv.h"
#include "mutex.h"
#include "common.h"
#include "list.hpp"
#include "non_copy_able.hpp"

class Buffer : public NonCopyAble, public BaseList::BaseNode {
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

private:
    uv_buf_t uv_buf_;
    Allocator & allocator_;

    std::atomic_long ref_;
    const size_t size_;
    size_t used_;

    /*
     * Start of the actual buffer, must remain the last
     */
    uint8_t buffer_[1];
};

class Buffer::Allocator : public NonCopyAble {
    friend class Buffer;
    typedef TNodeList<Buffer> BufferList;

public:
    explicit Allocator(size_t buffer_size, size_t max_free_buffers);
    virtual ~Allocator();

    Buffer * Allocate();

protected:
    void Flush();

private:
    void Release(Buffer * buffer);
    void DestroyBuffer(Buffer * buffer);

    virtual void OnBufferCreated();
    virtual void OnBufferAllocated();
    virtual void OnBufferReleased();
    virtual void OnBufferDestroyed();

private:
    const size_t buffer_size_;
    const size_t max_free_buffers_;

    BufferList free_list_;
    BufferList active_list_;
};

#endif