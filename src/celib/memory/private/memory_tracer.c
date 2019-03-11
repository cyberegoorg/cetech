#include <memory.h>
#include <stdlib.h>
#include <stdatomic.h>

#include <celib/memory/allocator.h>
#include <celib/api.h>

#include <celib/memory/memory.h>
#include <celib/macros.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/id.h>
#include "celib/memory/allocator.h"
#include "celib/log.h"

#define LOG_WHERE "memory"
#define CE_TRACE_MEMORY 0

#define _G ct_memory_global

#include "virt_alloc.inl"
#include "system_alloc.inl"

#define MAX_TRACER 256
#define MAX_TRACER_PTR 4096

typedef struct trace_item_t {
    void *ptr;
    const char *filename;
    int line;
} trace_item_t;

typedef struct memory_tracer_t {
    atomic_size_t allocated_size;
    trace_item_t items[MAX_TRACER_PTR];
} memory_tracer_t;


static struct _G {
    //memory tracer
    atomic_int tracer_n;
    memory_tracer_t tracer_inst[MAX_TRACER];
    ce_memory_tracer_t0 tracers[MAX_TRACER];
} _G = {};

////

static inline uint32_t _find_free_slot(ce_memory_tracer_o0 *inst,
                                       void *ptrs) {
    memory_tracer_t *tracer = (memory_tracer_t *) inst;

    uint64_t k = ((uint64_t) ptrs) * 11400714819323198549UL;

    const uint32_t idx_first = k % MAX_TRACER_PTR;
    uint32_t idx = idx_first;


    while ((tracer->items[idx].ptr != ((void *) 0)) &&
           (tracer->items[idx].ptr != ((void *) 1))) {
        idx = (idx + 1) % MAX_TRACER_PTR;

        if (idx == idx_first) {
            return UINT32_MAX;
        }
    }

    return idx;
}

static inline uint32_t _find_slot(ce_memory_tracer_o0 *inst,
                                  void *ptrs) {
    memory_tracer_t *tracer = (memory_tracer_t *) inst;

    uint64_t k = ((uint64_t) ptrs) * 11400714819323198549UL;

    const uint32_t idx_first = k % MAX_TRACER_PTR;
    uint32_t idx = idx_first;

    while ((tracer->items[idx].ptr != ((void *) 0)) &&
           (tracer->items[idx].ptr != ptrs)) {
        idx = (idx + 1) % MAX_TRACER_PTR;

        if (idx == idx_first) {
            return UINT32_MAX;
        }
    }

    return idx;
}

void trace_ptr(ce_memory_tracer_o0 *inst,
               void *ptr,
               size_t size,
               const char *filename,
               uint32_t line) {
    memory_tracer_t *tracer = (memory_tracer_t *) inst;
    atomic_fetch_add(&tracer->allocated_size, size);

    uint32_t slotidx = _find_free_slot(inst, ptr);

    if (slotidx == UINT32_MAX) {
        return;
    }

    trace_item_t *item = &tracer->items[slotidx];

    *item = (trace_item_t) {
            .ptr = ptr,
            .filename = filename,
            .line =line,
    };
}

void untrace_ptr(ce_memory_tracer_o0 *inst,
                 void *ptr,
                 size_t size) {
    memory_tracer_t *tracer = (memory_tracer_t *) inst;
    atomic_fetch_sub(&tracer->allocated_size, size);

    uint32_t slotidx = _find_slot(inst, ptr);
    if (slotidx == UINT32_MAX) {
        return;
    }
    trace_item_t *item = &tracer->items[slotidx];

    *item = (trace_item_t) {
            .ptr = (void *) 1,
            .filename = NULL,
            .line =0,
    };
}

static ce_memory_tracer_vt0 tracer_vt = {
        .trace_ptr = trace_ptr,
        .untrace_ptr = untrace_ptr,
};

ce_memory_tracer_t0 *create_tracer() {
    uint32_t idx = _G.tracer_n++;

    memory_tracer_t *inst = &_G.tracer_inst[idx];
    ce_memory_tracer_t0 *tracer = &_G.tracers[idx];

    *inst = (memory_tracer_t) {
            .items = {},
            .allocated_size = 0,
    };

    *tracer = (ce_memory_tracer_t0) {
            .inst = (ce_memory_tracer_o0 *) inst,
            .vt = &tracer_vt,
    };

    return tracer;
}

static struct ce_memory_tracer_a0 _tracer_api = {
        .create = create_tracer
};

struct ce_memory_tracer_a0 *ce_memory_tracer_a0 = &_tracer_api;

void memory_tracer_init(struct ce_api_a0 *api) {
    api->register_api(CE_MEMORY_TRACER_API, &_tracer_api, sizeof(_tracer_api));
}

void memory_tracer_shutdown() {
    for (int i = 0; i < _G.tracer_n; ++i) {
        memory_tracer_t *tracer = &_G.tracer_inst[i];

        if (tracer->allocated_size) {
            ce_log_a0->error(LOG_WHERE, "leak count %zu\n", tracer->allocated_size);
        }

//        for (int j = 0; j < MAX_TRACER_PTR; ++j) {
//            trace_item_t *item = &tracer->items[j];
//            if (!item->ptr) {
//                continue;
//            }
//
//            ce_log_a0->error(LOG_WHERE, "leak from %s:%d", item->filename, item->line);
//        }
    }

}
