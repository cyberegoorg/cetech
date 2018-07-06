#include <memory.h>
#include <stdlib.h>

#include <corelib/api_system.h>
#include <corelib/os.h>
#include <corelib/memory.h>
#include <corelib/macros.h>
#include "corelib/allocator.h"
#include "corelib/log.h"

#define LOG_WHERE "memory"
#define ALLOCATOR_WHERE "allocator"

static void *_reallocate(const struct ct_alloc *a,
                         void *ptr,
                         uint32_t size,
                         uint32_t align,
                         const char *filename,
                         uint32_t line) {
    CT_UNUSED(a);
    CT_UNUSED(align);
    CT_UNUSED(filename);
    CT_UNUSED(line);

    void *new_ptr = NULL;

    if (size) {
        new_ptr = realloc(ptr, size);
    } else {
        free(ptr);
    }

    return new_ptr;
}

static struct ct_alloc_fce alloc_fce = {
        .reallocate = _reallocate
};

static struct ct_alloc _allocator = {
        .inst = NULL,
        .call = &alloc_fce,
};


char *str_dup(const char *s,
              struct ct_alloc *allocator) {
    const uint32_t size = strlen(s) + 1;

    char *d = CT_ALLOC(allocator, char, size);
    CETECH_ASSERT("string", d != NULL);

    memset(d, '\0', sizeof(char) * size);
    strcpy(d, s);

    return d;
}

static struct ct_memory_a0 _api = {
        .system = &_allocator,
        .str_dup = str_dup,
};

struct ct_memory_a0 *ct_memory_a0 = &_api;

void memory_register_api(struct ct_api_a0 *api) {
    api->register_api("ct_memory_a0", &_api);
}

void memory_init() {

}

void memsys_shutdown() {
}



