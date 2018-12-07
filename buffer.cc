#include "buffer.h"
#include "exception.h"

Buffer::Buffer(Allocator & allocator, size_t size) : allocator_(allocator), ref_(1), size_(size), used_(0) {
    Empty();
}

uv_buf_t * Buffer::GetUVBuffer() const {
    return const_cast<uv_buf_t *>(&uv_buf_);
}

const uint8_t * Buffer::GetBuffer() const {
    return &buffer_[0];
}

size_t Buffer::GetUsed() const {
    return used_;
}

size_t Buffer::GetSize() const {
    return size_;
}

void Buffer::SetupRead() {
    if (0 == used_) {
        uv_buf_.base = reinterpret_cast<char *>(buffer_);
        uv_buf_.len = size_;
    } else {
        uv_buf_.base = reinterpret_cast<char *>(buffer_ + used_);
        uv_buf_.len = size_ - used_;
    }
}

void Buffer::SetupWrite(size_t offset) {
    assert(used_ > offset);
    uv_buf_.base = reinterpret_cast<char *>(buffer_ + offset);
    uv_buf_.len = used_ - offset;
}

void Buffer::AddData(const char * const data, size_t data_length) {
    assert(data_length < size_ - used_);
    memcpy(buffer_ + used_, data, data_length);
    used_ += data_length;
}

void Buffer::AddData(const uint8_t * const data, size_t data_length) {
    this->AddData(reinterpret_cast<const char *>(data), data_length);
}

void Buffer::AddData(char data) {
    this->AddData(&data, 1);
}

void Buffer::Use(size_t data_used) {
    used_ += data_used;
}

Buffer * Buffer::SplitBuffer(size_t bytes_to_remove) {
    Buffer * buffer = allocator_.Allocate();

    buffer->AddData(buffer_, bytes_to_remove);
    
    used_ -= bytes_to_remove;

    memmove(buffer_, buffer_ + bytes_to_remove, used_);

    return buffer;
}

void Buffer::Empty() {
    uv_buf_.base = reinterpret_cast<char *>(buffer_);
    uv_buf_.len = size_;

    used_ = 0;
}

void Buffer::AddRef() {
    ++ref_;
}

void Buffer::Release() {
    if (0 == --ref_) {
        allocator_.Release(this);
    }
}

void * Buffer::operator new(size_t object_size, size_t buffer_size) {
    return jc_malloc(object_size + buffer_size);
}

void Buffer::operator delete(void * object) {
    jc_free(object);
}

Buffer::Allocator::Allocator(size_t buffer_size, size_t max_free_buffers)
    : buffer_size_(buffer_size), max_free_buffers_(max_free_buffers) {

}

Buffer::Allocator::~Allocator() {
    Flush();
}

Buffer * Buffer::Allocator::Allocate() {
    Buffer * buffer = nullptr;

    if (!free_list_.IsEmpty()) {
        buffer = free_list_.PopLeft();
        buffer->AddRef();
    } else {
        buffer = new(buffer_size_)Buffer(*this, buffer_size_);
        if (!buffer) {
            throw BaseException("Buffer::Allocator::Allocate()", "Out of memory");
        }
        
        /*
         * Call to unqualified virtual function
         */
        OnBufferCreated();
    }

    active_list_.PushLeft(buffer);

    /*
     * call to unqualified virtual function
     */
    OnBufferAllocated();

    return buffer;
}

void Buffer::Allocator::Flush() {
    while (!active_list_.IsEmpty()) {
        /*
         * Call to unqualified virtual function
         */
        OnBufferReleased();

        DestroyBuffer(active_list_.PopLeft());
    }

    while (!free_list_.IsEmpty()) {
        DestroyBuffer(free_list_.PopLeft());
    }
}

void Buffer::Allocator::Release(Buffer * buffer) {
    if (!buffer) {
        throw BaseException("Buffer::Allocator::Release()", "buffer is null");
    }

    /*
     * Call to unqualified virtual function
     */
    OnBufferReleased();

    /*
     * unlink from the in use list
     */
    buffer->RemoveFromList();

    if (0 == max_free_buffers_ || free_list_.Count() < max_free_buffers_) {
        buffer->Empty();

        /*
         * Add to the free list
         */
        free_list_.PushLeft(buffer);
    } else {
        DestroyBuffer(buffer);
    }
}

void Buffer::Allocator::DestroyBuffer(Buffer * buffer) {
    delete buffer;

    /*
     * Call to unqualified virtual function
     */
    OnBufferDestroyed();
}

void Buffer::Allocator::OnBufferCreated() {

}

void Buffer::Allocator::OnBufferAllocated() {

}

void Buffer::Allocator::OnBufferReleased() {

}

void Buffer::Allocator::OnBufferDestroyed() {

}