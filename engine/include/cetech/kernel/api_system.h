//! \addtogroup Plugin
//! \{
#ifndef CETECH_API_H
#define CETECH_API_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Defines
//==============================================================================

#define CETECH_DECL_API(name) static struct name name = {0};
#define CETECH_GET_API(_api, name) name = *(struct name*) (_api)->first(#name).api

//==============================================================================
// Structs
//==============================================================================

struct ct_api_entry {
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

    struct ct_api_entry (*next)(struct ct_api_entry *entry);
};

struct ct_api_a0 *ct_api_get();

#ifdef __cplusplus
}
#endif

#endif //CETECH_API_H
//! \}