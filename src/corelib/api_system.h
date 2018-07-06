#ifndef CETECH_API_H
#define CETECH_API_H

#import <stdint.h>
#include <corelib/module.inl>

//==============================================================================
// Defines
//==============================================================================

#define CETECH_GET_API(_api, name) name = (struct name*) (_api)->first(#name).api

#define API_EBUS_NAME "api"


//==============================================================================
// Structs
//==============================================================================

struct ct_api_entry {
    uint32_t idx;
    void *entry;
    void *api;
};


typedef void (ct_api_on_add_t)(uint64_t name, void* api);

//==============================================================================
// Api
//==============================================================================

struct ct_api_a0 {
    void (*register_api)(const char *name,
                         void *api);

    int (*exist)(const char *name);

    struct ct_api_entry (*next)(struct ct_api_entry entry);

    struct ct_api_entry (*first)(const char *name);

    void (*register_on_add)(uint64_t name, ct_api_on_add_t *on_add);
};

CT_MODULE(ct_api_a0);

#endif //CETECH_API_H

