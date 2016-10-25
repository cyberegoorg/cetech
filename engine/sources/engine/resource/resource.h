#ifndef CETECH_RESOURCE_MANAGER_H
#define CETECH_RESOURCE_MANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/string/stringid.h>
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
// Structs and typedefs
//==============================================================================

struct compilator_api {
    void (*add_dependency)(const char *who_filname,
                           const char *depend_on_filename);
};

typedef int (*resource_compilator_t)(
        const char *filename,
        struct vio *source_vio,
        struct vio *build_vio,
        struct compilator_api *compilator_api);

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

void resource_compiler_register(stringid64_t type,
                                resource_compilator_t compilator);

void resource_compiler_compile_all();

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   stringid64_t type,
                                   stringid64_t name);

const char *resource_compiler_get_source_dir();

const char *resource_compiler_get_core_dir();

int resource_compiler_get_build_dir(char *build_dir,
                                    size_t max_len,
                                    const char *platform);

int resource_compiler_get_tmp_dir(char *tmp_dir,
                                  size_t max_len,
                                  const char *platform);

int resource_compiler_external_join(char *output,
                                    u32 max_len,
                                    const char *name);

void resource_compiler_create_build_dir();

//==============================================================================
// Package interface
//==============================================================================

void package_load(stringid64_t name);

void package_unload(stringid64_t name);

int package_is_loaded(stringid64_t name);

void package_flush(stringid64_t name);

#endif //CETECH_RESOURCE_MANAGER_H
