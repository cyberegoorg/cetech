//! \defgroup Resource
//! Resource system
//! \{
#ifndef CETECH_RESOURCE_TYPES_H
#define CETECH_RESOURCE_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <engine/application/application.h>
#include "celib/string/stringid.h"
#include "celib/containers/array.h"
#include "celib/types.h"
#include "celib/filesystem/vio_types.h"
#include "engine/config/cvar.h"


//==============================================================================
// Callback typedef
//==============================================================================

typedef void *(*resource_loader_t)(struct vio *input,
                                   struct cel_allocator *allocator);

typedef void  (*resource_online_t)(stringid64_t name,
                                   void *data);

typedef void  (*resource_offline_t)(stringid64_t name,
                                    void *data);

typedef void  (*resource_unloader_t)(void *new_data,
                                     struct cel_allocator *allocator);

typedef void *(*resource_reloader_t)(stringid64_t name,
                                     void *old_data,
                                     void *new_data,
                                     struct cel_allocator *allocator);

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

struct ResourceApiV1 {
    void (*set_autoload)(int enable);

    void (*register_type)(stringid64_t type,
                          resource_callbacks_t callbacks);

    void (*load)(void **loaded_data,
                 stringid64_t type,
                 stringid64_t *names,
                 size_t count,
                 int force);

    void (*add_loaded)(stringid64_t type,
                       stringid64_t *names,
                       void **resource_data,
                       size_t count);

    void (*load_now)(stringid64_t type,
                     stringid64_t *names,
                     size_t count);

    void (*unload)(stringid64_t type,
                   stringid64_t *names,
                   size_t count);

    void (*reload)(stringid64_t type,
                   stringid64_t *names,
                   size_t count);

    void (*reload_all)();

    int (*can_get)(stringid64_t type,
                   stringid64_t names);

    int (*can_get_all)(stringid64_t type,
                       stringid64_t *names,
                       size_t count);

    void *(*get)(stringid64_t type,
                 stringid64_t names);

    int (*type_name_string)(char *str,
                            size_t max_len,
                            stringid64_t type,
                            stringid64_t name);

    void (*compiler_register)(stringid64_t type,
                              resource_compilator_t compilator);

    void (*compiler_compile_all)();

    int (*compiler_get_filename)(char *filename,
                                 size_t max_ken,
                                 stringid64_t type,
                                 stringid64_t name);

    int (*compiler_get_build_dir)(char *build_dir,
                                  size_t max_len,
                                  const char *platform);

    int (*compiler_get_tmp_dir)(char *tmp_dir,
                                size_t max_len,
                                const char *platform);

    int (*compiler_external_join)(char *output,
                                  u32 max_len,
                                  const char *name);

    void (*compiler_create_build_dir)(struct ConfigApiV1 config, struct ApplicationApiV1 app);

    const char *(*compiler_get_core_dir)();

    const char *(*compiler_get_source_dir)();
};

//==============================================================================
// Package interface
//==============================================================================

struct PackageApiV1 {
    void (*load)(stringid64_t name);

    void (*unload)(stringid64_t name);

    int (*is_loaded)(stringid64_t name);

    void (*flush)(stringid64_t name);
};

#endif //CETECH_RESOURCE_TYPES_H
//! |}