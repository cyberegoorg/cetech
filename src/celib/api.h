// # API

#ifndef CE_API_H
#define CE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib_types.h"

#include "murmur.h"

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
typedef struct ce_api_entry_t0 {
    uint32_t idx;
    void *entry;
    void *api;
} ce_api_entry_t0;


typedef void (ce_api_on_add_t0)(uint64_t name,
                                void *api);

struct ce_api_a0 {
    void (*register_api)(uint64_t name_id,
                         void *api,
                         uint32_t size);

    void (*add_impl)(uint64_t name_id,
                     void *api,
                     uint32_t size);

    int (*exist)(const char *name);

    ce_api_entry_t0 (*first)(uint64_t name);

    ce_api_entry_t0 (*next)(ce_api_entry_t0 entry);

    void (*register_on_add)(uint64_t name,
                            ce_api_on_add_t0 *on_add);
};


CE_MODULE(ce_api_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_API_H
