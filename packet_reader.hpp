#ifndef __LIBCOMMON_PACKET_READER_HPP__
#define __LIBCOMMON_PACKET_READER_HPP__

#include "common.h"

class PacketReader
{
public:
    PacketReader() : mem_ptr_(nullptr), end_ptr_(nullptr), offset_(nullptr), data_end_(nullptr) {}
    PacketReader(uint8_t * buf, size_t size) : mem_ptr_(buf), end_ptr_(buf + size), offset_(buf), data_end_(buf + size) {}

    // ��ȡ�ڴ�ռ�Ĵ�С
    inline size_t GetSize() { return end_ptr_ - mem_ptr_; }
    // ��ȡ���ݰ��ĵ�ǰ��Ч���ݳ���
    inline size_t GetLength() { return data_end_ - mem_ptr_; }
    // ��ȡ�Ե�ǰ��дָ�뿪ʼ���Զ�ȡ��ʣ���ֽ���
    inline size_t GetAvaliableLength() { return data_end_ - offset_; }
    // ��ȡ���ݰ���ǰ��дλ���ֽ�ƫ����
    inline size_t GetPosition() { return offset_ - mem_ptr_; }
    // ��ȡ���ݰ������ڴ�ָ��
    inline uint8_t * GetMemoryPtr() { return mem_ptr_; }
    // ��ȡ���ݰ������ڴ�ĵ�ǰƫ��ָ��
    inline uint8_t * GetOffsetPtr() { return offset_; }
    // ��ȡָ��ƫ������ָ��
    inline uint8_t * GetPositionPtr(size_t pos) { return mem_ptr_ + pos; }

    // �������ݰ���ǰ��дλ���ֽ�ƫ����������µ�ƫ�����ȵ�ǰ�ڴ�鳤��Ҫ���������µ�ƫ��λ�ò�������ǰ�ڴ�鳤��
    virtual size_t SetPosition(size_t new_pos)
    {
        size_t mem_size = GetSize();
        if (new_pos > mem_size) new_pos = mem_size;
        offset_ = mem_ptr_ + new_pos;
        return new_pos;
    }

    // ���ڵ�ǰ��дָ��ƫ��λ�õ���ƫ����
    virtual size_t AdjustOffset(int64_t adjust_offset)
    {
        offset_ += adjust_offset;
        if (offset_ < mem_ptr_) offset_ = mem_ptr_;
        else if (offset_ > end_ptr_) offset_ = end_ptr_;
        return offset_ - mem_ptr_;
    }

    // ��ȡԭ������
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

    // ��ȡ����������
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

    // ��ȡ�ַ������ݣ��ַ��������ݸ�ʽΪ��[2�ֽ��ַ���������][�ַ����ֽ����ݣ��ַ���������65536����][�ַ�����ֹ�ַ�0]
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
    // ��ȡ�ַ�����������
    template<typename LENGTH>
    size_t RawReadStringLength(char * str, size_t len)
    {
        assert(len > 0);
        size_t avaliable = GetAvaliableLength(), read_len = 0, str_len = 0;
        
        if (avaliable >= sizeof(LENGTH))
        {
            read_len = str_len = *(reinterpret_cast<LENGTH *>(offset_));

            // �����ַ�����������
            offset_ += sizeof(LENGTH);
            avaliable -= sizeof(LENGTH);

            // ����ʵ�ʶ�ȡ���ȣ�����Խ��
            if (read_len > len) read_len = len;
            if (read_len > avaliable) read_len = avaliable;
            if (read_len > 0) ReadBinary(reinterpret_cast<uint8_t *>(str), read_len);

            // �����ȡ���ַ�����������������ָ�����ַ������ȣ�����Ҫ����û�ж�ȡ�Ĳ��֣��Ա������������ȷ�Ĵ����ݰ��ж�ȡ����������
            if (str_len > read_len) offset_ += (str_len - read_len);

            // ������ֹ�ַ�0
            ++offset_;

            // offset���ܳ���data_end�ķ�Χ
            if (offset_ > data_end_) offset_ = data_end_;
        }

        // �����ֹ�ַ�0
        if (read_len >= len) read_len = len - 1;
        str[read_len] = 0;

        // �����ַ�������
        return str_len;
    }

    // ��ȡ�ַ�������
    template<typename LENGTH>
    const char * RawReadStringPtr()
    {
        size_t avaliable = GetAvaliableLength();

        if (avaliable >= sizeof(LENGTH) + sizeof(char))
        {
            size_t str_len = *(reinterpret_cast<LENGTH *>(offset_));

            // ������ݰ������㹻���ַ������ݿռ�
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