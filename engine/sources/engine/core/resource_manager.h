#ifndef CETECH_RESOURCE_MANAGER_H
#define CETECH_RESOURCE_MANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/stringid/types.h"
#include "celib/memory/types.h"
#include "celib/os/vio_types.h"

//==============================================================================
// Callback typedef
//==============================================================================

typedef void *(*resource_loader_t)(struct vio *input,
                                   struct allocator *allocator);

typedef void  (*resource_online_t)(stringid64_t name,
                                   void *data);

typedef void  (*resource_offline_t)(stringid64_t name,
                                    void *data);

typedef void  (*resource_unloader_t)(void *new_data,
                                     struct allocator *allocator);

typedef void *(*resource_reloader_t)(stringid64_t name,
                                     void *old_data,
                                     void *new_data,
                                     struct allocator *allocator);


//==============================================================================
// Structs
//==============================================================================

typedef struct {
    resource_loader_t loader;
    resource_unloader_t unloader;
    resource_online_t online;
    resource_offline_t offline;
    resource_reloader_t reloader;
} resource_callbacks_t;

//==============================================================================
// Resource interface
//==============================================================================

int resource_init(int stage);

void resource_shutdown();

void resource_set_autoload(int enable);

void resource_register_type(stringid64_t type,
                            resource_callbacks_t callbacks);

void resource_load(void **loaded_data,
                   stringid64_t type,
                   stringid64_t *names,
                   size_t count,
                   int force);

void resource_add_loaded(stringid64_t type,
                         stringid64_t *names,
                         void **resource_data,
                         size_t count);

void resource_load_now(stringid64_t type,
                       stringid64_t *names,
                       size_t count);

void resource_unload(stringid64_t type,
                     stringid64_t *names,
                     size_t count);

void resource_reload(stringid64_t type,
                     stringid64_t *names,
                     size_t count);

void resource_reload_all();

int resource_can_get(stringid64_t type,
                     stringid64_t names);

int resource_can_get_all(stringid64_t type,
                         stringid64_t *names,
                         size_t count);

void *resource_get(stringid64_t type,
                   stringid64_t names);

int resource_type_name_string(char *str,
                              size_t max_len,
                              stringid64_t type,
                              stringid64_t name);

//==============================================================================
// Package interface
//==============================================================================

void package_load(stringid64_t name);

void package_unload(stringid64_t name);

int package_is_loaded(stringid64_t name);

void package_flush(stringid64_t name);

#endif //CETECH_RESOURCE_MANAGER_H
