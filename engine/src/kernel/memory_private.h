#ifndef CETECH__MEMORY_H
#define CETECH__MEMORY_H

namespace memory {

    void init(int scratch_buffer_size);

    void register_api(struct ct_api_a0 *api);

    void memsys_shutdown();

    struct ct_allocator *memsys_main_allocator();

    struct ct_allocator *memsys_main_scratch_allocator();


    struct allocator_trace_entry {
        void *ptr;
        char *stacktrace;
        char used;
    };


//==============================================================================
// Allocator
//==============================================================================

    void allocator_trace_pointer(struct allocator_trace_entry *entries,
                                 uint64_t max_entries,
                                 void *p);

    void allocator_stop_trace_pointer(struct allocator_trace_entry *entries,
                                      uint64_t max_entries,
                                      void *p);

    void allocator_check_trace(struct allocator_trace_entry *entries,
                               uint64_t max_entries);

//==============================================================================
// Malloc ct_allocator
//==============================================================================


    struct ct_allocator *malloc_allocator_create();

    void malloc_allocator_destroy(struct ct_allocator *a);


//==============================================================================
// Scratch ct_allocator
//==============================================================================

    struct ct_allocator *scratch_allocator_create(struct ct_allocator *backing,
                                               int size);

    void scratch_allocator_destroy(struct ct_allocator *a);
}

static const void *pointer_align_forward(const void *p,
                                         uint32_t align) {
    uintptr_t pi = (uintptr_t) p;
    const uint32_t mod = pi % align;
    if (mod)
        pi += (align - mod);
    return (void *) pi;
}

struct Header {
    uint32_t size;
};

static const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

static void *data_pointer(struct Header *header,
                          uint32_t align) {
    const void *p = header + 1;
    return (void *) pointer_align_forward(p, align);
}

static struct Header *header(void *data) {
    uint32_t *p = (uint32_t *) data;

    while (p[-1] == HEADER_PAD_VALUE)
        --p;

    return (struct Header *) p - 1;
}

static void fill(struct Header *header,
                 void *data,
                 uint32_t size) {
    header->size = size;
    uint32_t *p = (uint32_t *) (header + 1);
    while (p < (uint32_t *) data)
        *p++ = HEADER_PAD_VALUE;
}


#endif //CETECH__MEMORY_H
