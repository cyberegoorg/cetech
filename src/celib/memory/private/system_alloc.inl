#ifndef CETECH_SYSTEM_ALLOC_INL
#define CETECH_SYSTEM_ALLOC_INL

#include "../memory_tracer.h"

typedef struct system_allocator_t {
    ce_mem_tracer_t0 *tracer;
} system_allocator_t;

static system_allocator_t _system_allocator_inst;

static ce_mem_tracer_t0 *_memory_tracer(const ce_alloc_o0 *_a) {
    system_allocator_t *a = (system_allocator_t *) _a;
    return a->tracer;
}

static void *_reallocate(const ce_alloc_o0 *_a,
                         void *ptr,
                         size_t size,
                         size_t old_size,
                         size_t align,
                         const char *filename,
                         uint32_t line) {
    CE_UNUSED(align);

    void *new_ptr = NULL;

    new_ptr = realloc(ptr, size);

    system_allocator_t *a = (system_allocator_t *) _a;
    ce_mem_tracer_t0 *tracer = a->tracer;

    if (tracer) {
        tracer->vt->untrace_ptr(tracer->inst, ptr);

        if (size) {
            tracer->vt->trace_ptr(tracer->inst, new_ptr, size, filename, line);
        }
    }

    return new_ptr;
}


static struct ce_alloc_vt0 system_vt = {
        .reallocate = _reallocate,
        .memory_tracer = _memory_tracer,
};

static struct ce_alloc_t0 _system_allocator = {
        .inst = (ce_alloc_o0 *) &_system_allocator_inst,
        .vt = &system_vt,
};


#endif //CETECH_SYSTEM_ALLOC_INL
