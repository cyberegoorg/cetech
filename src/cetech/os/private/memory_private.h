#ifndef CETECH__MEMORY_H
#define CETECH__MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

void memory_init();

void register_api(struct ct_api_a0 *api);

void memsys_shutdown();

struct cel_alloc *memsys_main_allocator();

struct allocator_trace_entry {
    void *ptr;
    //char *stacktrace;
    char used;
};


//==============================================================================
// Allocator
//==============================================================================

void allocator_trace_pointer(struct allocator_trace_entry *entries,
                             uint32_t max_entries,
                             void *p);

void allocator_stop_trace_pointer(struct allocator_trace_entry *entries,
                                  uint32_t max_entries,
                                  void *p);

void allocator_check_trace(struct allocator_trace_entry *entries,
                           uint32_t max_entries);

//==============================================================================
// Malloc cel_alloc
//==============================================================================


struct cel_alloc *malloc_allocator_create();

void malloc_allocator_destroy(struct cel_alloc *a);

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

#ifdef __cplusplus
}
#endif

#endif //CETECH__MEMORY_H
