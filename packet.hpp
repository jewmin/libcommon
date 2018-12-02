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

    // �����ڴ�������С
    inline void SetAllocSize(size_t size) {
        alloc_size_ = size;
    }

    // ��ȡ��ǰ���峤���µĿ����ֽ���
    inline size_t GetWritableLength() {
        return end_ptr_ - offset_;
    }

    // �������ݰ���ǰ��дλ���ֽ�ƫ����������µ�ƫ�����ȵ�ǰ�ڴ�鳤��Ҫ����������SetLengthһ�������ڴ�����չ����
    size_t SetPosition(size_t new_pos) override {
        if (new_pos > GetSize()) {
            Realloc(new_pos);
        }
        offset_ = mem_ptr_ + new_pos;

        // �����дָ�볬������ָ���λ�ã����������ָ��Ϊ��дָ���λ��
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
        return new_pos;
    }

    // ���ڵ�ǰ��дָ��ƫ��λ�õ���ƫ����
    // adjust_offsetҪ������ƫ������С���������ʾ���ڴ濪ʼ������������ƫ�ƣ�
    // �����ڻ����Ƶ������ƫ��λ�ñ�����ڵ����ڴ濪ͷ������������ƫ�Ƴ��������ڴ泤�ȣ�����Զ������ڴ浽���������λ��
    size_t AdjustOffset(int64_t adjust_offset) override {
        offset_ += adjust_offset;
        if (offset_ < mem_ptr_) {
            offset_ = mem_ptr_;
        } else if (offset_ > end_ptr_) {
            Realloc(offset_ - mem_ptr_);
        }

        // �����дָ�볬������ָ���λ�ã����������ָ��Ϊ��дָ���λ��
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
        return offset_ - mem_ptr_;
    }

    // �������ݳ���
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

    // �ͷ��ڴ�
    void Free() {
        if (mem_ptr_ && size_ > 0) {
            jc_free(mem_ptr_);
            mem_ptr_ = end_ptr_ = offset_ = data_end_ = nullptr;
            size_ = 0;
        }
    }

    // д��ԭ������
    template<typename T>
    inline void WriteAtom(const T & data) {
        size_t mem_size = GetWritableLength();
        if (mem_size < sizeof(data)) {
            Realloc(GetSize() + sizeof(data));
        }
        *(reinterpret_cast<T *>(offset_)) = data;
        offset_ += sizeof(data);

        // �����дָ�볬������ָ���λ�ã����������ָ��Ϊ��дָ���λ��
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
    }

    // д�����������
    inline void WriteBinary(const uint8_t * buf, size_t size) {
        size_t mem_size = GetWritableLength();
        if (mem_size < size) {
            Realloc(GetSize() + size);
        }
        memcpy(offset_, buf, size);
        offset_ += size;

        // �����дָ�볬������ָ���λ�ã����������ָ��Ϊ��дָ���λ��
        if (offset_ > data_end_) {
            data_end_ = offset_;
        }
    }

    // д���ַ������ݣ��ַ��������ݸ�ʽΪ��[2�ֽ��ַ���������][�ַ����ֽ����ݣ��ַ���������65536����][�ַ�����ֹ�ַ�0]
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
    // �����趨���ݰ��ڴ�ռ��С
    void Realloc(size_t new_size) {
        new_size += alloc_size_;
        uint8_t * old_mem = mem_ptr_;
        uint8_t * new_mem = static_cast<uint8_t *>(jc_malloc(new_size));
        size_t offset = GetPosition();
        size_t length = GetLength();
        if (length > 0) {
            memcpy(new_mem, mem_ptr_, std::min<size_t>(new_size, length));
        }

        // ��������ָ��
        mem_ptr_ = new_mem;
        end_ptr_ = mem_ptr_ + new_size;
        offset_ = mem_ptr_ + offset;
        data_end_ = mem_ptr_ + length;

        // ����ԭ�������ڴ�
        if (old_mem && size_ > 0) {
            jc_free(old_mem);
        }

        // ���÷����ڴ��С
        size_ = new_size;
    }

    // д���ַ������ݣ��ַ��������ݸ�ʽΪ��[2�ֽ��ַ���������][�ַ����ֽ����ݣ��ַ���������65536����][�ַ�����ֹ�ַ�0]
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
    size_t alloc_size_; // �ڴ�������С
    size_t size_; // ʹ���ڴ������������ڴ�
};

#endif