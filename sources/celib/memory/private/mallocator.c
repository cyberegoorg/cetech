#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "../allocator.h"
#include "../ialocator.h"

#include "../../string/string.h"
#include "../../utils/utils.h"
#include "../../log/log.h"
#include "../../errors/errors.h"
#include "../memory.h"

#define LOG_WHERE "allocator.mallocator"

static const char* _TYPE_NAME = "mallocator";

struct mallocator {
    struct iallocator i;
};

void *_mallocator_malloc(Alloc_t allocator, size_t size) {
    CE_ASSERT(LOG_WHERE, allocator != NULL);
    CE_ASSERT(LOG_WHERE, size != 0);

    void *p = memory_malloc(size);

    return p;
}

void _mallocator_free(Alloc_t allocator, void *ptr) {
    free(ptr);
}

Alloc_t mallocator_create() {
    struct mallocator *m = (struct mallocator *) memory_malloc(sizeof(struct mallocator));
    CE_ASSERT(LOG_WHERE, m != NULL);

    *m = (struct mallocator) {0};

    m->i.alloc = _mallocator_malloc;
    m->i.free = _mallocator_free;

#if defined(CETECH_DEBUG)
    m->i.type_name = _TYPE_NAME;
#endif

    return m;
}


void mallocator_destroy(Alloc_t allocator) {
    CE_ASSERT(LOG_WHERE, allocator != NULL);

    alloc_destroy(allocator);
    memory_free(allocator);
}

void mallocator_destructor(void **ptr) {
    mallocator_destroy(*ptr);
}
