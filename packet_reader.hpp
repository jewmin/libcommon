#ifndef __LIBCOMMON_PACKET_READER_HPP__
#define __LIBCOMMON_PACKET_READER_HPP__

#include "common.h"

class PacketReader
{
public:
    PacketReader() : mem_ptr_(nullptr), end_ptr_(nullptr), offset_(nullptr), data_end_(nullptr) {}
    PacketReader(uint8_t * buf, size_t size) : mem_ptr_(buf), end_ptr_(buf + size), offset_(buf), data_end_(buf + size) {}

    // 获取内存空间的大小
    inline size_t GetSize() { return end_ptr_ - mem_ptr_; }
    // 获取数据包的当前有效数据长度
    inline size_t GetLength() { return data_end_ - mem_ptr_; }
    // 获取自当前读写指针开始可以读取的剩余字节数
    inline size_t GetAvaliableLength() { return data_end_ - offset_; }
    // 获取数据包当前读写位置字节偏移量
    inline size_t GetPosition() { return offset_ - mem_ptr_; }
    // 获取数据包数据内存指针
    inline uint8_t * GetMemoryPtr() { return mem_ptr_; }
    // 获取数据包数据内存的当前偏移指针
    inline uint8_t * GetOffsetPtr() { return offset_; }
    // 获取指定偏移量的指针
    inline uint8_t * GetPositionPtr(size_t pos) { return mem_ptr_ + pos; }

    // 设置数据包当前读写位置字节偏移量，如果新的偏移量比当前内存块长度要大，则限制新的偏移位置不超过当前内存块长度
    virtual size_t SetPosition(size_t new_pos)
    {
        size_t mem_size = GetSize();
        if (new_pos > mem_size) new_pos = mem_size;
        offset_ = mem_ptr_ + new_pos;
        return new_pos;
    }

    // 基于当前读写指针偏移位置调整偏移量
    virtual size_t AdjustOffset(int64_t adjust_offset)
    {
        offset_ += adjust_offset;
        if (offset_ < mem_ptr_) offset_ = mem_ptr_;
        else if (offset_ > end_ptr_) offset_ = end_ptr_;
        return offset_ - mem_ptr_;
    }

    // 读取原子数据
    template<typename T>
    inline T ReadAtom()
    {
        T value;
        size_t avaliable = GetAvaliableLength();
        if (avaliable >= sizeof(T))
        {
            value = *(reinterpret_cast<T *>(offset_));
            offset_ += sizeof(T);
        }
        else
        {
            memset(&value, 0, sizeof(value));
            offset_ += avaliable;
        }
        return value;
    }

    // 读取二进制数据
    inline size_t ReadBinary(uint8_t * buf, size_t size)
    {
        size_t avaliable = GetAvaliableLength();
        if (avaliable >= size)
        {
            memcpy(buf, offset_, size);
        }
        else
        {
            memset(buf, 0, size);
            size = avaliable;
        }
        offset_ += size;
        return size;
    }

    // 读取字符串数据，字符串的数据格式为：[2字节字符长度数据][字符串字节数据，字符串长度在65536以内][字符串终止字符0]
    inline size_t ReadString(char * str, size_t len)
    {
        return RawReadStringLength<uint16_t>(str, len);
    }

    inline const char * ReadString()
    {
        return RawReadStringPtr<uint16_t>();
    }

    template<typename T>
    inline PacketReader & operator >> (T & value)
    {
        if (sizeof(value) <= sizeof(int32_t)) value = ReadAtom<T>();
        else ReadBinary(reinterpret_cast<uint8_t *>(&value), sizeof(value));
        return *this;
    }

    inline PacketReader & operator >> (const char * & str)
    {
        str = RawReadStringPtr<uint16_t>();
        return *this;
    }

protected:
    // 读取字符串到缓冲区
    template<typename LENGTH>
    size_t RawReadStringLength(char * str, size_t len)
    {
        assert(len > 0);
        size_t avaliable = GetAvaliableLength(), read_len = 0, str_len = 0;
        
        if (avaliable >= sizeof(LENGTH))
        {
            read_len = str_len = *(reinterpret_cast<LENGTH *>(offset_));

            // 跳过字符串长度数据
            offset_ += sizeof(LENGTH);
            avaliable -= sizeof(LENGTH);

            // 计算实际读取长度，避免越界
            if (read_len > len) read_len = len;
            if (read_len > avaliable) read_len = avaliable;
            if (read_len > 0) ReadBinary(reinterpret_cast<uint8_t *>(str), read_len);

            // 如果读取的字符串长度少于数据中指定的字符串长度，则需要跳过没有读取的部分，以便接下来可以正确的从数据包中读取后续的数据
            if (str_len > read_len) offset_ += (str_len - read_len);

            // 跳过终止字符0
            ++offset_;

            // offset不能超过data_end的范围
            if (offset_ > data_end_) offset_ = data_end_;
        }

        // 添加终止字符0
        if (read_len >= len) read_len = len - 1;
        str[read_len] = 0;

        // 返回字符串长度
        return str_len;
    }

    // 读取字符串数据
    template<typename LENGTH>
    const char * RawReadStringPtr()
    {
        size_t avaliable = GetAvaliableLength();

        if (avaliable >= sizeof(LENGTH) + sizeof(char))
        {
            size_t str_len = *(reinterpret_cast<LENGTH *>(offset_));

            // 如果数据包中有足够的字符串数据空间
            if (avaliable >= str_len + sizeof(LENGTH) + sizeof(char))
            {
                const char * str = reinterpret_cast<char *>(offset_ + sizeof(LENGTH));
                offset_ += str_len + sizeof(LENGTH) + sizeof(char);
                return str;
            }
        }

        return nullptr;
    }

protected:
    uint8_t * mem_ptr_;
    uint8_t * end_ptr_;
    uint8_t * offset_;
    uint8_t * data_end_;
};

#endif