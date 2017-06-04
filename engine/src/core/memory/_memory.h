#ifndef CETECH__MEMORY_H
#define CETECH__MEMORY_H

namespace memory {

    void memsys_init(int scratch_buffer_size);

    void memsys_init_api(struct api_v0 *api);

    void memsys_shutdown();

    struct allocator *_memsys_main_allocator();

    struct allocator *_memsys_main_scratch_allocator();


    struct allocator_trace_entry {
        void *ptr;
        char *stacktrace;
        char used;
    };

//==============================================================================
// Malloc
//==============================================================================

    void *malloc(size_t size);

    void free(void *ptr);

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
// Malloc allocator
//==============================================================================


    struct allocator *malloc_allocator_create();

    void malloc_allocator_destroy(struct allocator *a);


//==============================================================================
// Scratch allocator
//==============================================================================

    struct allocator *scratch_allocator_create(struct allocator *backing,
                                               int size);

    void scratch_allocator_destroy(struct allocator *a);

}


#endif //CETECH__MEMORY_H
