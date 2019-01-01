// # API

#ifndef CE_API_H
#define CE_API_H

#include <stdint.h>

#include "murmur_hash.inl"
#include "module.inl"

#define CE_API_API \
    CE_ID64_0("ce_api_a0", 0x95b23cc231103112ULL)

// Init api
#define CE_INIT_API(_api, name) \
    name = (struct name*) (_api)->first(ce_hash_murmur2_64(#name, strlen(#name), 0)).api


// Api entry
//
// - *api* - Api pointer
// - *idx* - private.
// - *entry* - private.
struct ce_api_entry {
    uint32_t idx;
    void *entry;
    void *api;
};


typedef void (ce_api_on_add_t)(uint64_t name, void* api);

struct ce_api_a0 {
    void (*register_api)(uint64_t name_id,
                         void *api);

    int (*exist)(const char *name);

    struct ce_api_entry (*first)(uint64_t name);

    struct ce_api_entry (*next)(struct ce_api_entry entry);

    void (*register_on_add)(uint64_t name, ce_api_on_add_t *on_add);
};


CE_MODULE(ce_api_a0);

#endif //CE_API_H
