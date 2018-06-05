#ifndef CETECH__MEMORY_H
#define CETECH__MEMORY_H



#include <stdlib.h>

void memory_init();

void register_api(struct ct_api_a0 *api);

void memsys_shutdown();

struct ct_alloc *memsys_main_allocator();

struct allocator_trace_entry {
    void *ptr;
    //char *stacktrace;
    char used;
};


//==============================================================================
// Malloc ct_alloc
//==============================================================================


struct ct_alloc *malloc_allocator_create();

void malloc_allocator_destroy(struct ct_alloc *a);

const void *pointer_align_forward(const void *p,
                                  uint32_t align);

struct Header {
    uint32_t size;
};

static const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

void *data_pointer(struct Header *header,
                   uint32_t align);

struct Header *header(void *data);

void fill(struct Header *header,
          void *data,
          uint32_t size);


#endif //CETECH__MEMORY_H
