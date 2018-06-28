#ifndef CETECH_API_H
#define CETECH_API_H


#include <corelib/module.inl>
#import <stdint.h>

//==============================================================================
// Defines
//==============================================================================

#define CETECH_GET_API(_api, name) name = (struct name*) (_api)->first(#name).api


//==============================================================================
// Structs
//==============================================================================

struct ct_api_entry {
    uint32_t idx;
    void *entry;
    void *api;
};


//==============================================================================
// Api
//==============================================================================

struct ct_api_a0 {
    void (*register_api)(const char *name,
                         void *api);

    int (*exist)(const char *name);

    struct ct_api_entry (*first)(const char *name);

    struct ct_api_entry (*next)(struct ct_api_entry entry);
};

CT_MODULE(ct_api_a0);

#endif //CETECH_API_H

