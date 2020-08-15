#include "SDString.h"
#include "Allocator.h"

namespace Common {

SDString::SDString() : SDString("", 0) {
}

SDString::SDString(const size_t count, const i8 & ch) {
	sds_ = NewSDS(count);
	if (count > 0) {
		std::memset(sds_, ch, count);
	}
}

SDString::SDString(const i8 * s) {
	size_t count = s ? std::strlen(s) : 0;
	sds_ = NewSDS(count);
	if (count > 0 && s) {
		std::memcpy(sds_, s, count);
	}
}

SDString::SDString(const i8 * s, const size_t count) {
	sds_ = NewSDS(count);
	if (count > 0 && s) {
		std::memcpy(sds_, s, count);
	}
}

SDString::SDString(const SDString & other, const size_t pos, size_t count) : SDString(std::move(other.SubStr(pos, count))) {
}

SDString::SDString(SDString && other) {
	sds_ = other.sds_;
	other.sds_ = nullptr;
}

SDString::SDString(const SDString & other) : SDString(other.sds_, other.Size()) {
}

SDString & SDString::operator=(const i8 * s) {
	return Assign(s);
}

SDString & SDString::operator=(const i8 & ch) {
	return Assign(1, ch);
}

SDString & SDString::operator=(SDString && other) {
	if (this != std::addressof(other)) {
		DelSDS(sds_);
		sds_ = other.sds_;
		other.sds_ = nullptr;
	}
	return *this;
}

SDString & SDString::operator=(const SDString & other) {
	if (this != std::addressof(other)) {
		Assign(other);
	}
	return *this;
}

SDString::~SDString() {
	DelSDS(sds_);
}

SDString & SDString::Assign(const SDString & other) {
	return Assign(other.sds_, other.Size());	
}

SDString & SDString::Assign(const SDString & other, const size_t pos, size_t count = SDString::npos) {
	assert(pos <= other.Size());
	count = std::min(count, other.Size() - pos);
	return Assign(other.sds_ + pos, count);
}

SDString & SDString::Assign(const i8 * s) {
	size_t count = s ? std::strlen(s) : 0;
	return Assign(s, count);
}

SDString & SDString::Assign(const i8 * s, const size_t count) {
	if (count <= AllocSize()) {
		if (count > 0 && s) {
			std::memcpy(sds_, s, count);
		}
		sds_[count] = 0;
		SetSize(count);
		return *this;
	}

	DelSDS(sds_);
	sds_ = NewSDS(count);
	if (s) {
		std::memcpy(sds_, s, count);
	}
	return *this;
}

SDString & SDString::Assign(const size_t count, const i8 & ch) {
	if (count <= AllocSize()) {
		if (count > 0) {
			std::memset(sds_, ch, count);
		}
		sds_[count] = 0;
		SetSize(count);
		return *this;
	}

	DelSDS(sds_);
	sds_ = NewSDS(count);
	std::memset(sds_, ch, count);
	return *this;
}

SDString & SDString::Append(const SDString & other) {
	return Append(other.sds_, other.Size());
}

SDString & SDString::Append(const SDString & other, const size_t pos, size_t count = SDString::npos) {
	assert(pos <= other.Size());
	count = std::min(count, other.Size() - pos);
	return Append(other.sds_ + pos, count);
}

SDString & SDString::Append(const i8 * s) {
	size_t count = s ? std::strlen(s) : 0;
	return Append(s, count);
}

SDString & SDString::Append(const i8 * s, const size_t count) {
	if (count > 0 && s) {
		if (count <= AvailSize()) {
			size_t pos = Size();
			std::memcpy(sds_ + pos, s, count);
			sds_[pos + count] = 0;
			SetSize(pos + count);
			return *this;
		}

		size_t old_size = Size();
		size_t new_size = count < min_alloc ? old_size + min_alloc : old_size + count;
		i8 * new_sds = NewSDS(new_size);
		std::memcpy(new_sds, sds_, old_size);
		std::memcpy(new_sds + old_size, s, count);
		DelSDS(sds_);
		sds_ = new_sds;
		SetSize(old_size + count);
	}
	return *this;
}

SDString & SDString::Append(const size_t count, const i8 & ch) {
	if (count > 0) {
		if (count <= AvailSize()) {
			size_t pos = Size();
			std::memset(sds_ + pos, ch, count);
			sds_[pos + count] = 0;
			SetSize(pos + count);
			return *this;
		}

		size_t old_size = Size();
		size_t new_size = count < min_alloc ? old_size + min_alloc : old_size + count;
		i8 * new_sds = NewSDS(new_size);
		std::memcpy(new_sds, sds_, old_size);
		std::memset(new_sds + old_size, ch, count);
		DelSDS(sds_);
		sds_ = new_sds;
		SetSize(old_size + count);
	}
	return *this;
}

i8 * SDString::NewSDS(const size_t count) {
	u8 type = Type(count);
	size_t hdr_size = HdrSize(type);
	void * data = jc_malloc(hdr_size + count + 1);
	std::memset(data, 0, hdr_size + count + 1);
	i8 * sds = static_cast<i8 *>(data) + hdr_size;
	switch (type) {
		case SDS_TYPE_8: {
			struct sdshdr8 * hdr = SDS_HDR(8, sds);
			hdr->len = count;
			hdr->alloc = count;
			hdr->flags = type;
			break;
		}
		case SDS_TYPE_16: {
			struct sdshdr16 * hdr = SDS_HDR(16, sds);
			hdr->len = count;
			hdr->alloc = count;
			hdr->flags = type;
			break;
		}
		case SDS_TYPE_32: {
			struct sdshdr32 * hdr = SDS_HDR(32, sds);
			hdr->len = count;
			hdr->alloc = count;
			hdr->flags = type;
			break;
		}
		case SDS_TYPE_64: {
			struct sdshdr64 * hdr = SDS_HDR(64, sds);
			hdr->len = count;
			hdr->alloc = count;
			hdr->flags = type;
			break;
		}
	}
	return sds;
}

void SDString::DelSDS(i8 * sds) {
	if (sds) {
		jc_free(sds - HdrSize(sds[-1]));
	}
}

SDString SDString::Format(const i8 * format, ...) {
	va_list args;
	va_start(args, format);
	SDString s = FormatVa(format, args);
	va_end(args);
	return s;
}

SDString SDString::FormatVa(const i8 * format, va_list args) {
	size_t size = 1024;
	i8 * buffer = new i8[size];

	while (true) {
		va_list args_copy;
#ifdef _WIN32
		args_copy = args;
#else
		va_copy(args_copy, args);
#endif
		i32 n = std::vsnprintf(buffer, size, format, args_copy);
		va_end(args_copy);

		if (n > -1 && static_cast<size_t>(n) < size) {
			SDString s(buffer);
			delete [] buffer;
			return s;
		}

		size = n > -1 ? n + 1 : size << 1;
		delete [] buffer;
		buffer = new i8[size];
	}
}

void SDString::TrimStartAndEndInline() {
	TrimEndInline();
	TrimStartInline();
}

SDString SDString::TrimStartAndEnd() const & {
	SDString s(*this);
	s.TrimStartAndEndInline();
	return s;
}

SDString SDString::TrimStartAndEnd() && {
	SDString s(std::move(*this));
	s.TrimStartAndEndInline();
	return s;
}

void SDString::TrimStartInline() {
	size_t pos = 0;
	size_t size = Size();
	while (pos < size && std::isspace(sds_[pos])) {
		++pos;
	}
	RemoveAt(0, pos);
}

SDString SDString::TrimStart() const & {
	SDString s(*this);
	s.TrimStartInline();
	return s;
}

SDString SDString::TrimStart() && {
	SDString s(std::move(*this));
	s.TrimStartInline();
	return s;
}

void SDString::TrimEndInline() {
	size_t size = Size();
	size_t end = size;
	while (end > 0 && std::isspace(sds_[end - 1])) {
		--end;
	}
	RemoveAt(end, size - end);
}

SDString SDString::TrimEnd() const & {
	SDString s(*this);
	s.TrimEndInline();
	return s;
}

SDString SDString::TrimEnd() && {
	SDString s(std::move(*this));
	s.TrimEndInline();
	return s;
}

}