#ifndef __LIBCOMMON_PACKET_HPP__
#define __LIBCOMMON_PACKET_HPP__

#include <algorithm>

#include "common.h"
#include "packet_reader.hpp"
#include "non_copy_able.hpp"

class Packet : public PacketReader {
public:
    Packet() : PacketReader(), alloc_size_(0), size_(0) {

    }

    Packet(uint8_t * buf, size_t size) : PacketReader(buf, size), alloc_size_(0), size_(0) {
        data_end_ = mem_ptr_;
    }

    virtual ~Packet() {
        Free();
    }

    // 设置内存增长大小
    inline void SetAllocSize(size_t size) {
        alloc_size_ = size;
    }

    // 获取当前缓冲长度下的可用字节数
    inline size_t GetWritableLength() {
        return end_ptr_ - offset_;
    }

    // 设置数据包当前读写位置字节偏移量，如果新的偏移量比当前内存块长度要大，则函数会与SetLength一样进行内存块的扩展操作
    size_t SetPosition(size_t new_pos) override {
        if (new_pos > GetSize()) {
            Realloc(new_pos);
        }
        offset_ = mem_ptr_ + new_pos;

        // 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
        return new_pos;
    }

    // 基于当前读写指针偏移位置调整偏移量
    // adjust_offset要调整的偏移量大小，负数则表示向内存开始处调整（降低偏移）
    // 函数内会限制调整后的偏移位置必须大于等于内存开头，如果调整后的偏移超过现有内存长度，则会自动增长内存到欲求调整的位置
    size_t AdjustOffset(int64_t adjust_offset) override {
        offset_ += adjust_offset;
        if (offset_ < mem_ptr_) {
            offset_ = mem_ptr_;
        } else if (offset_ > end_ptr_) {
            Realloc(offset_ - mem_ptr_);
        }

        // 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
        return offset_ - mem_ptr_;
    }

    // 设置数据长度
    inline void SetLength(size_t new_length) {
        if (new_length > GetSize()) {
            Realloc(new_length);
        }
        data_end_ = mem_ptr_ + new_length;
        if (offset_ > data_end_) {
            offset_ = data_end_;
        }
    }

    inline size_t Reserve(size_t new_size) {
        size_t mem_size = GetSize();
        if (mem_size < new_size) {
            Realloc(new_size);
        }
        return mem_size;
    }

    // 释放内存
    void Free() {
        if (mem_ptr_ && size_ > 0) {
            jc_free(mem_ptr_);
            mem_ptr_ = end_ptr_ = offset_ = data_end_ = nullptr;
            size_ = 0;
        }
    }

    // 写入原子数据
    template<typename T>
    inline void WriteAtom(const T & data) {
        size_t mem_size = GetWritableLength();
        if (mem_size < sizeof(data)) {
            Realloc(GetSize() + sizeof(data));
        }
        *(reinterpret_cast<T *>(offset_)) = data;
        offset_ += sizeof(data);

        // 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
    }

    // 写入二进制数据
    inline void WriteBinary(const uint8_t * buf, size_t size) {
        size_t mem_size = GetWritableLength();
        if (mem_size < size) {
            Realloc(GetSize() + size);
        }
        memcpy(offset_, buf, size);
        offset_ += size;

        // 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
    }

    // 写入字符串数据，字符串的数据格式为：[2字节字符长度数据][字符串字节数据，字符串长度在65536以内][字符串终止字符0]
    inline void WriteString(const char * str, size_t len = -1) {
        RawWriteStringLength<uint16_t>(str, len);
    }

    template<typename T>
    inline Packet & operator << (const T & value) {
        if (sizeof(value) <= sizeof(int32_t)) {
            WriteAtom<T>(value);
        } else {
            WriteBinary(reinterpret_cast<const uint8_t *>(&value), sizeof(value));
        }
        return *this;
    }

    inline Packet & operator << (const char * value) {
        WriteString(value, strlen(value));
        return *this;
    }

    inline Packet & operator << (char * value) {
        WriteString(value, strlen(value));
        return *this;
    }

protected:
    // 重新设定数据包内存空间大小
    void Realloc(size_t new_size) {
        new_size += alloc_size_;
        uint8_t * old_mem = mem_ptr_;
        uint8_t * new_mem = static_cast<uint8_t *>(jc_malloc(new_size));
        size_t offset = GetPosition();
        size_t length = GetLength();
        if (length > 0) {
            memcpy(new_mem, mem_ptr_, std::min<size_t>(new_size, length));
        }

        // 重设数据指针
        mem_ptr_ = new_mem;
        end_ptr_ = mem_ptr_ + new_size;
        offset_ = mem_ptr_ + offset;
        data_end_ = mem_ptr_ + length;

        // 销毁原有数据内存
        if (old_mem && size_ > 0) {
            jc_free(old_mem);
        }

        // 设置分配内存大小
        size_ = new_size;
    }

    // 写入字符串数据，字符串的数据格式为：[2字节字符长度数据][字符串字节数据，字符串长度在65536以内][字符串终止字符0]
    template<typename LENGTH>
    void RawWriteStringLength(const char * str, size_t len) {
        if (!str) {
            str = "";
        }

        if ((size_t)-1 == len) {
            len = str ? (LENGTH)strlen(str) : 0;
        } else {
            len = std::min<size_t>(len, strlen(str));
        }

        WriteAtom<LENGTH>((LENGTH)len);
        WriteBinary(reinterpret_cast<const uint8_t *>(str), len);
        WriteAtom<char>(0);
    }

protected:
    size_t alloc_size_; // 内存增长大小
    size_t size_; // 使用内存分配器分配的内存
};

#endif