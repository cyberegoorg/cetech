#ifndef CETECH_BAGRAPH_H
#define CETECH_BAGRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

typedef struct ce_ba_graph_t {
    ce_hash_t graph_map;
    uint64_t **before;
    uint64_t **after;
    uint64_t *name;
    uint64_t *input;
    uint64_t *output;
    uint64_t *output_no_dep;
} ce_ba_graph_t;

static inline void _ce_bag_remove(uint64_t *a,
                                  uint64_t item) {
    uint64_t n = ce_array_size(a);
    for (int i = 0; i < n; ++i) {
        if (item != a[i]) {
            continue;
        }

        a[i] = a[n - 1];
        ce_array_pop_back(a);
        break;
    }
}

static inline uint64_t ce_bag_get_or_create(ce_ba_graph_t *sg,
                                            uint64_t name,
                                            ce_alloc_t0 *alloc) {
    if (!ce_hash_contain(&sg->graph_map, name)) {
        uint64_t idx = ce_array_size(sg->after);

        ce_array_push(sg->after, NULL, alloc);
        ce_array_push(sg->before, NULL, alloc);
        ce_array_push(sg->name, name, alloc);

        ce_hash_add(&sg->graph_map, name, idx, alloc);
    }

    uint64_t idx = ce_hash_lookup(&sg->graph_map, name, UINT64_MAX);
    return idx;
}


static inline void ce_bag_add(ce_ba_graph_t *sg,
                              uint64_t name,
                              const uint64_t *before,
                              uint64_t before_n,
                              const uint64_t *after,
                              uint64_t after_n,
                              ce_alloc_t0 *alloc) {
    uint64_t idx = ce_bag_get_or_create(sg, name, alloc);

    for (int b = 0; b < before_n; ++b) {
        uint64_t b_name = before[b];
        ce_array_push(sg->before[idx], b_name, alloc);

        uint64_t idx = ce_bag_get_or_create(sg, b_name, alloc);
        ce_array_push(sg->after[idx], name, alloc);
    }

    for (int a = 0; a < after_n; ++a) {
        uint64_t a_name = after[a];
        ce_array_push(sg->after[idx], a_name, alloc);

        uint64_t idx = ce_bag_get_or_create(sg, a_name, alloc);
        ce_array_push(sg->before[idx], name, alloc);
    }
}

static inline void ce_bag_clean(ce_ba_graph_t *sg) {
    ce_array_clean(sg->output_no_dep);
    ce_array_clean(sg->output);
    ce_array_clean(sg->input);
    ce_array_clean(sg->name);

    uint64_t n = ce_array_size(sg->after);
    for (int i = 0; i < n; ++i) {
        ce_array_clean(sg->after);
        ce_array_clean(sg->before);
    }

    ce_hash_clean(&sg->graph_map);
}

static inline void ce_bag_build(ce_ba_graph_t *sg,
                                ce_alloc_t0 *alloc) {
    uint64_t *input_systems = NULL;

    uint64_t name_n = ce_array_size(sg->name);
    for (int j = 0; j < name_n; ++j) {

        if (ce_array_size(sg->before[j])) {
            continue;
        }

        ce_array_push(input_systems, sg->name[j], alloc);
    }

    while (ce_array_size(input_systems)) {
        uint64_t item_name = ce_array_back(input_systems);
        ce_array_pop_back(input_systems);

        ce_array_insert(sg->output, 0, item_name, alloc);

        uint64_t dep_idx = ce_hash_lookup(&sg->graph_map, item_name,
                                          UINT64_MAX);

        uint64_t *dep_affter = sg->after[dep_idx];
        uint64_t dep_affter_n = ce_array_size(dep_affter);

        for (int i = 0; i < dep_affter_n; ++i) {
            uint64_t after_name = dep_affter[i];
            uint64_t after_idx = ce_hash_lookup(&sg->graph_map, after_name,
                                                UINT64_MAX);

            uint64_t *before = sg->before[after_idx];

            _ce_bag_remove(before, item_name);

            uint64_t before_n = ce_array_size(before);

            if (!before_n) {
                ce_array_push(input_systems, after_name, alloc);
            }
        }

        ce_array_clean(dep_affter);
    }
}

#ifdef __cplusplus
};
#endif

#endif //CETECH_BAGRAPH_H
