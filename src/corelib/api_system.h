// # API

#ifndef CETECH_API_H
#define CETECH_API_H

#include <stdint.h>

#include "murmur_hash.inl"
#include "module.inl"

// Init api
#define CT_INIT_API(_api, name) \
    name = (struct name*) (_api)->first(ct_hash_murmur2_64(#name, strlen(#name), 0)).api


// Api entry
//
// - *api* - Api pointer
// - *idx* - private.
// - *entry* - private.
struct ct_api_entry {
    uint32_t idx;
    void *entry;
    void *api;
};


typedef void (ct_api_on_add_t)(uint64_t name, void* api);

struct ct_api_a0 {

    void (*register_api)(const char *name,
                         void *api);

    int (*exist)(const char *name);

    struct ct_api_entry (*next)(struct ct_api_entry entry);

    struct ct_api_entry (*first)(uint64_t name);

    void (*register_on_add)(uint64_t name, ct_api_on_add_t *on_add);
};


CT_MODULE(ct_api_a0);

#endif //CETECH_API_H

