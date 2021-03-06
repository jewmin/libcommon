/*
 * MIT License
 *
 * Copyright (c) 2019 jewmin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef Common_SDString_INCLUDED
#define Common_SDString_INCLUDED

#include "Common.h"
#include "CObject.h"

namespace Common {

#pragma pack(1)

struct sdshdr8 {
	u8 len;
	u8 alloc;
	u8 flags;
};

struct sdshdr16 {
	u16 len;
	u16 alloc;
	u8 flags;
};

struct sdshdr32 {
	u32 len;
	u32 alloc;
	u8 flags;
};

struct sdshdr64 {
	u64 len;
	u64 alloc;
	u8 flags;
};

#pragma pack()

#define SDS_TYPE_8		1
#define SDS_TYPE_16		2
#define SDS_TYPE_32		3
#define SDS_TYPE_64		4
#define SDS_TYPE_MASK	7
#define SDS_HDR(T, s)	((struct sdshdr##T *)((s) - (sizeof(struct sdshdr##T))))

class COMMON_EXTERN SDString : public CObject {
public:
	SDString();
	SDString(const size_t count, const i8 & ch);
	explicit SDString(const i8 * s);
	SDString(const SDString & other, const size_t pos, size_t count = SDString::npos);
	SDString(SDString && other);
	SDString(const SDString & other);
	SDString & operator=(const i8 * s);
	SDString & operator=(const i8 & ch);
	SDString & operator=(SDString && other);
	SDString & operator=(const SDString & other);
	~SDString();

	bool Empty() const;
	size_t Size() const;
	size_t AllocSize() const;
	size_t AvailSize() const;

	void Clear();
	void RemoveAt(size_t pos, size_t count = 1);
	SDString SubStr(const size_t pos = 0, size_t count = SDString::npos) const;
	size_t Find(const i8 * s, size_t pos = 0) const;
	size_t RFind(const i8 * s, size_t pos = SDString::npos) const;

	void TrimStartAndEndInline();
	SDString TrimStartAndEnd() const &;
	SDString TrimStartAndEnd() &&;
	void TrimStartInline();
	SDString TrimStart() const &;
	SDString TrimStart() &&;
	void TrimEndInline();
	SDString TrimEnd() const &;
	SDString TrimEnd() &&;

	SDString & Assign(const SDString & other);
	SDString & Assign(const SDString & other, const size_t pos, size_t count = SDString::npos);
	SDString & Assign(const i8 * s);
	SDString & Assign(const i8 * s, const size_t count);
	SDString & Assign(const size_t count, const i8 & ch);

	SDString & Append(const SDString & other);
	SDString & Append(const SDString & other, const size_t pos, size_t count = SDString::npos);
	SDString & Append(const i8 * s);
	SDString & Append(const i8 * s, const size_t count);
	SDString & Append(const size_t count, const i8 & ch);

	SDString operator+(const i8 * s);
	SDString operator+(const i8 & ch);
	SDString operator+(const SDString & other);

	SDString & operator+=(const i8 * s);
	SDString & operator+=(const i8 & ch);
	SDString & operator+=(const SDString & other);

	i8 & operator[](const size_t pos);
	const i8 & operator[](const size_t pos) const;
	const i8 * operator*() const;
	bool operator==(const i8 * s);
	bool operator==(const SDString & other);
	bool operator==(const i8 * s) const;
	bool operator==(const SDString & other) const;
	bool operator!=(const i8 * s);
	bool operator!=(const SDString & other);
	bool operator!=(const i8 * s) const;
	bool operator!=(const SDString & other) const;
	bool operator<(const i8 * s);
	bool operator<(const SDString & other);
	bool operator<(const i8 * s) const;
	bool operator<(const SDString & other) const;
	bool operator<=(const i8 * s);
	bool operator<=(const SDString & other);
	bool operator<=(const i8 * s) const;
	bool operator<=(const SDString & other) const;
	bool operator>(const i8 * s);
	bool operator>(const SDString & other);
	bool operator>(const i8 * s) const;
	bool operator>(const SDString & other) const;
	bool operator>=(const i8 * s);
	bool operator>=(const SDString & other);
	bool operator>=(const i8 * s) const;
	bool operator>=(const SDString & other) const;

	static size_t HdrSize(const u8 type);
	static SDString Format(const i8 * format, ...);
	static SDString FormatVa(const i8 * format, va_list args);

	static const size_t npos = -1;

protected:
	SDString(const i8 * s, const size_t count);
	u8 Type(const size_t count) const;
	void SetSize(const size_t count);
	size_t Find(const i8 * s, size_t pos, size_t n) const;
	size_t RFind(const i8 * s, size_t pos, size_t n) const;

private:
	i8 * NewSDS(const size_t count);
	void DelSDS(i8 * sds);

private:
	i8 * sds_;
};

inline u8 SDString::Type(const size_t count) const {
	if (count < 256) {
		return SDS_TYPE_8;
	} else if (count < 65536) {
		return SDS_TYPE_16;
	} else if (count < 4294967296) {
		return SDS_TYPE_32;
	} else {
		return SDS_TYPE_64;
	}
}

inline bool SDString::Empty() const {
	return Size() == 0;
}

inline size_t SDString::Size() const {
	switch (sds_[-1] & SDS_TYPE_MASK) {
		case SDS_TYPE_8:
			return SDS_HDR(8, sds_)->len;
		case SDS_TYPE_16:
			return SDS_HDR(16, sds_)->len;
		case SDS_TYPE_32:
			return SDS_HDR(32, sds_)->len;
		case SDS_TYPE_64:
			return SDS_HDR(64, sds_)->len;
	}
	return 0;
}

inline size_t SDString::HdrSize(const u8 type) {
	switch (type & SDS_TYPE_MASK) {
		case SDS_TYPE_8:
			return sizeof(struct sdshdr8);
		case SDS_TYPE_16:
			return sizeof(struct sdshdr16);
		case SDS_TYPE_32:
			return sizeof(struct sdshdr32);
		case SDS_TYPE_64:
			return sizeof(struct sdshdr64);
	}
	return 0;
}

inline size_t SDString::AllocSize() const {
	switch (sds_[-1] & SDS_TYPE_MASK) {
		case SDS_TYPE_8:
			return SDS_HDR(8, sds_)->alloc;
		case SDS_TYPE_16:
			return SDS_HDR(16, sds_)->alloc;
		case SDS_TYPE_32:
			return SDS_HDR(32, sds_)->alloc;
		case SDS_TYPE_64:
			return SDS_HDR(64, sds_)->alloc;
	}
	return 0;
}

inline size_t SDString::AvailSize() const {
	switch (sds_[-1] & SDS_TYPE_MASK) {
		case SDS_TYPE_8: {
			struct sdshdr8 * hdr = SDS_HDR(8, sds_);
			return hdr->alloc - hdr->len;
		}
		case SDS_TYPE_16: {
			struct sdshdr16 * hdr = SDS_HDR(16, sds_);
			return hdr->alloc - hdr->len;
		}
		case SDS_TYPE_32: {
			struct sdshdr32 * hdr = SDS_HDR(32, sds_);
			return hdr->alloc - hdr->len;
		}
		case SDS_TYPE_64: {
			struct sdshdr64 * hdr = SDS_HDR(64, sds_);
			return hdr->alloc - hdr->len;
		}
	}
	return 0;
}

inline void SDString::SetSize(const size_t count) {
	switch (sds_[-1] & SDS_TYPE_MASK) {
		case SDS_TYPE_8:
			SDS_HDR(8, sds_)->len = static_cast<u8>(count);
			break;
		case SDS_TYPE_16:
			SDS_HDR(16, sds_)->len = static_cast<u16>(count);
			break;
		case SDS_TYPE_32:
			SDS_HDR(32, sds_)->len = static_cast<u32>(count);
			break;
		case SDS_TYPE_64:
			SDS_HDR(64, sds_)->len = static_cast<u64>(count);
			break;
	}
}

inline void SDString::Clear() {
	SetSize(0);
	sds_[0] = 0;
}

inline void SDString::RemoveAt(size_t pos, size_t count) {
	if (count > 0) {
		size_t size = Size();
		assert(pos >= 0 && pos + count <= size);
		size_t num_move = size - pos - count;
		if (num_move > 0) {
			std::memmove(sds_ + pos, sds_ + pos + count, num_move);
		}
		sds_[size - count] = 0;
		SetSize(size - count);
	}
}

inline SDString SDString::SubStr(const size_t pos, size_t count) const {
	assert(pos <= Size());
	count = std::min(count, Size() - pos);
	return SDString(sds_ + pos, count);
}

inline SDString SDString::operator+(const i8 * s) {
	size_t size = Size();
	size_t other_size = s ? std::strlen(s) : 0;
	if (0 == size) {
		return SDString(s, other_size);
	}
	if (0 == other_size) {
		return SDString(sds_, size);
	}

	SDString new_s(nullptr, size + other_size);
	std::memcpy(new_s.sds_, sds_, size);
	std::memcpy(new_s.sds_ + size, s, other_size);
	return new_s;
}

inline SDString SDString::operator+(const i8 & ch) {
	size_t size = Size();
	if (0 == size) {
		return SDString(1, ch);
	}

	SDString new_s(nullptr, size + 1);
	std::memcpy(new_s.sds_, sds_, size);
	std::memset(new_s.sds_ + size, ch, 1);
	return new_s;
}

inline SDString SDString::operator+(const SDString & other) {
	size_t size = Size();
	size_t other_size = other.Size();
	if (0 == size) {
		return SDString(other.sds_, other_size);
	}
	if (0 == other_size) {
		return SDString(sds_, size);
	}

	SDString new_s(nullptr, size + other_size);
	std::memcpy(new_s.sds_, sds_, size);
	std::memcpy(new_s.sds_ + size, other.sds_, other_size);
	return new_s;
}

inline SDString & SDString::operator+=(const i8 * s) {
	return Append(s);
}

inline SDString & SDString::operator+=(const i8 & ch) {
	return Append(1, ch);
}

inline SDString & SDString::operator+=(const SDString & other) {
	return Append(other);
}

inline i8 & SDString::operator[](const size_t pos) {
	assert(pos <= Size());
	return sds_[pos];
}

inline const i8 & SDString::operator[](const size_t pos) const {
	assert(pos <= Size());
	return sds_[pos];
}

inline const i8 * SDString::operator*() const {
	return sds_;
}

inline bool SDString::operator==(const i8 * s) {
	return std::strcmp(sds_, s) == 0;
}

inline bool SDString::operator==(const SDString & other) {
	return std::strcmp(sds_, other.sds_) == 0;
}

inline bool SDString::operator==(const i8 * s) const {
	return std::strcmp(sds_, s) == 0;
}

inline bool SDString::operator==(const SDString & other) const {
	return std::strcmp(sds_, other.sds_) == 0;
}

inline bool SDString::operator!=(const i8 * s) {
	return std::strcmp(sds_, s) != 0;
}

inline bool SDString::operator!=(const SDString & other) {
	return std::strcmp(sds_, other.sds_) != 0;
}

inline bool SDString::operator!=(const i8 * s) const {
	return std::strcmp(sds_, s) != 0;
}

inline bool SDString::operator!=(const SDString & other) const {
	return std::strcmp(sds_, other.sds_) != 0;
}

inline bool SDString::operator<(const i8 * s) {
	return std::strcmp(sds_, s) < 0;
}

inline bool SDString::operator<(const SDString & other) {
	return std::strcmp(sds_, other.sds_) < 0;
}

inline bool SDString::operator<(const i8 * s) const {
	return std::strcmp(sds_, s) < 0;
}

inline bool SDString::operator<(const SDString & other) const {
	return std::strcmp(sds_, other.sds_) < 0;
}

inline bool SDString::operator<=(const i8 * s) {
	return std::strcmp(sds_, s) <= 0;
}

inline bool SDString::operator<=(const SDString & other) {
	return std::strcmp(sds_, other.sds_) <= 0;
}

inline bool SDString::operator<=(const i8 * s) const {
	return std::strcmp(sds_, s) <= 0;
}

inline bool SDString::operator<=(const SDString & other) const {
	return std::strcmp(sds_, other.sds_) <= 0;
}

inline bool SDString::operator>(const i8 * s) {
	return std::strcmp(sds_, s) > 0;
}

inline bool SDString::operator>(const SDString & other) {
	return std::strcmp(sds_, other.sds_) > 0;
}

inline bool SDString::operator>(const i8 * s) const {
	return std::strcmp(sds_, s) > 0;
}

inline bool SDString::operator>(const SDString & other) const {
	return std::strcmp(sds_, other.sds_) > 0;
}

inline bool SDString::operator>=(const i8 * s) {
	return std::strcmp(sds_, s) >= 0;
}

inline bool SDString::operator>=(const SDString & other) {
	return std::strcmp(sds_, other.sds_) >= 0;
}

inline bool SDString::operator>=(const i8 * s) const {
	return std::strcmp(sds_, s) >= 0;
}

inline bool SDString::operator>=(const SDString & other) const {
	return std::strcmp(sds_, other.sds_) >= 0;
}

}

#endif