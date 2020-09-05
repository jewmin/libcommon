#include "Allocator.h"

typedef struct {
	jc_malloc_func local_malloc;
	jc_realloc_func local_realloc;
	jc_calloc_func local_calloc;
	jc_free_func local_free;
} jc_allocator_t;

static jc_allocator_t jc_allocator = {
	std::malloc,
	std::realloc,
	std::calloc,
	std::free,
};

void * jc_malloc(std::size_t size) {
	return jc_allocator.local_malloc(size);
}

void * jc_realloc(void * ptr, std::size_t size) {
	return jc_allocator.local_realloc(ptr, size);
}

void * jc_calloc(std::size_t count, std::size_t size) {
	return jc_allocator.local_calloc(count, size);
}

void jc_free(void * ptr) {
	jc_allocator.local_free(ptr);
}

bool jc_replace_allocator(jc_malloc_func malloc_func, jc_realloc_func realloc_func, jc_calloc_func calloc_func, jc_free_func free_func) {
	if (nullptr == malloc_func || nullptr == realloc_func || nullptr == calloc_func || nullptr == free_func) {
		return false;
	}

	jc_allocator.local_malloc = malloc_func;
	jc_allocator.local_realloc = realloc_func;
	jc_allocator.local_calloc = calloc_func;
	jc_allocator.local_free = free_func;
	return true;
}