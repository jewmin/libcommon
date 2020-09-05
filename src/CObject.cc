#include "CObject.h"
#include "Allocator.h"

namespace Common {

CObject::CObject() {
}

CObject::~CObject() {
}

void * CObject::operator new(size_t object_size) {
	return jc_malloc(object_size);
}

void CObject::operator delete(void * object, size_t object_size) {
	jc_free(object);
}

void * CObject::operator new(size_t, void * object) {
	return object;
}

void CObject::operator delete(void *, void *) {
}

void * CObject::operator new[](std::size_t object_size) {
	return jc_malloc(object_size);
}

void CObject::operator delete[](void * object, std::size_t object_size) {
	jc_free(object);
}

}