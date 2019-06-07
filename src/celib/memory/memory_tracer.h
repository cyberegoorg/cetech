#ifndef CE_MEMORY_TRACER_H
#define CE_MEMORY_TRACER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_MEMORY_TRACER_API \
    CE_ID64_0("ce_memory_tracer_a0", 0x9aa78aa7346e8a72ULL)

typedef struct ce_alloc_t0 ce_alloc_t0;

typedef struct ce_memory_tracer_o0 ce_memory_tracer_o0;

typedef struct ce_memory_tracer_vt0 {
    void (*trace_ptr)(ce_memory_tracer_o0 *inst,
                      void *ptr,
                      size_t size,
                      const char *filename,
                      uint32_t line);

    void (*untrace_ptr)(ce_memory_tracer_o0 *inst,
                        void *ptr,
                        size_t size);
} ce_memory_tracer_vt0;

typedef struct ce_memory_tracer_t0 {
    ce_memory_tracer_o0 *inst;
    ce_memory_tracer_vt0 *vt;
} ce_memory_tracer_t0;

struct ce_memory_tracer_a0 {
    ce_memory_tracer_t0 *(*create)();

    void (*destroy)(ce_memory_tracer_t0 *tracer);
};

CE_MODULE(ce_memory_tracer_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_MEMORY_TRACER_H
