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
// Typedefs
//==============================================================================

//! Resource loader
//! \param input Input vio
//! \param allocator Allocator
//! \return Resource data
typedef void *(*resource_loader_t)(struct vio *input,
                                   struct cel_allocator *allocator);

//! Resource online callback
//! \param name Resource name
//! \param data Resource data
typedef void  (*resource_online_t)(stringid64_t name,
                                   void *data);

//! Resource offline callback
//! \param name Resource name
//! \param data Resource data
typedef void  (*resource_offline_t)(stringid64_t name,
                                    void *data);

//! Resource unload callback
//! \param new_data Resource data
//! \param allocator Allocator
typedef void  (*resource_unloader_t)(void *new_data,
                                     struct cel_allocator *allocator);

//! Resource reloader
//! \param name Resource name
//! \param old_data Resource old data
//! \param new_data Resource new data
//! \param allocator Allocator
typedef void *(*resource_reloader_t)(stringid64_t name,
                                     void *old_data,
                                     void *new_data,
                                     struct cel_allocator *allocator);

//! Resource compilator fce
//! \param filename Source filename
//! \param source_vio Source vio
//! \param build_vio Build vio
//! \param compilator_api Compilator api
typedef int (*resource_compilator_t)(
        const char *filename,
        struct vio *source_vio,
        struct vio *build_vio,
        struct compilator_api *compilator_api);


//==============================================================================
// Structs
//==============================================================================

//! Resource callbacks
typedef struct {
    resource_loader_t loader;     //!< Loader
    resource_unloader_t unloader; //!< Unloader
    resource_online_t online;     //!< Online
    resource_offline_t offline;   //!< Offline
    resource_reloader_t reloader; //!< Reloader
} resource_callbacks_t;


//! Compilator api
struct compilator_api {
    //! Add dependency
    //! \param who_filname Source filename
    //! \param depend_on_filename Depend on this file
    void (*add_dependency)(const char *who_filname,
                           const char *depend_on_filename);
};


//==============================================================================
// Api
//==============================================================================

//! Resource API V0
struct ResourceApiV0 {
    //! Enable autoload feature
    //! \param enable Enable
    void (*set_autoload)(int enable);

    //! Register resource type
    //! \param type Type
    //! \param callbacks Callbacks
    void (*register_type)(stringid64_t type,
                          resource_callbacks_t callbacks);

    //! Load resources
    //! \param loaded_data Loaded data array
    //! \param type Type
    //! \param names Names
    //! \param count Names count
    //! \param force Force load
    void (*load)(void **loaded_data,
                 stringid64_t type,
                 stringid64_t *names,
                 size_t count,
                 int force);

    //! Add loaded resourcess
    //! \param type Types
    //! \param names Names
    //! \param resource_data Resource data array
    //! \param count Resouce count
    void (*add_loaded)(stringid64_t type,
                       stringid64_t *names,
                       void **resource_data,
                       size_t count);


    //! Load resource (now)
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    void (*load_now)(stringid64_t type,
                     stringid64_t *names,
                     size_t count);

    //! Unload resources
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    void (*unload)(stringid64_t type,
                   stringid64_t *names,
                   size_t count);

    //! Reload resources
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    void (*reload)(stringid64_t type,
                   stringid64_t *names,
                   size_t count);

    //! Reload all loaded resource
    void (*reload_all)();

    //! Is resorce loaded?
    //! \param type Type
    //! \param names Name
    //! \return 1 if is loaded else 0
    int (*can_get)(stringid64_t type,
                   stringid64_t names);

    //! Is all resorces loaded?
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    //! \return 1 if is loaded else 0
    int (*can_get_all)(stringid64_t type,
                       stringid64_t *names,
                       size_t count);

    //! Get resouce data
    //! \param type Type
    //! \param names Name
    //! \return Resource data or NULL if resource is not loaded
    void *(*get)(stringid64_t type,
                 stringid64_t names);

    //! Type, Name => string
    //! \param str Result string
    //! \param max_len Max stirng len
    //! \param type Type
    //! \param name Name
    //! \return 1 if ok else 0
    int (*type_name_string)(char *str,
                            size_t max_len,
                            stringid64_t type,
                            stringid64_t name);

    //! Register resource compiler
    //! \param type Type
    //! \param compilator Compilator fce
    void (*compiler_register)(stringid64_t type,
                              resource_compilator_t compilator);

    //! Compile all resource in source dir
    void (*compiler_compile_all)();

    //! Type, Name => filename
    //! \param filename Result filename
    //! \param max_ken Max filename size
    //! \param type Type
    //! \param name Name
    //! \return 1 if ok else 0
    int (*compiler_get_filename)(char *filename,
                                 size_t max_ken,
                                 stringid64_t type,
                                 stringid64_t name);

    //! Get build dir
    //! \param build_dir Build dir
    //! \param max_len Max build dir len
    //! \param platform Platform
    //! \return 1 if ok else 0
    int (*compiler_get_build_dir)(char *build_dir,
                                  size_t max_len,
                                  const char *platform);

    //! Get build tmp dir
    //! \param tmp_dir Tmp dir
    //! \param max_len Max build dir len
    //! \param platform Platform
    //! \return 1 if ok else 0
    int (*compiler_get_tmp_dir)(char *tmp_dir,
                                size_t max_len,
                                const char *platform);

    //! Join tool path
    //! \param output Tmp dir
    //! \param max_len Max len
    //! \param name Tool name
    //! \return 1 if ok else 0
    int (*compiler_external_join)(char *output,
                                  u32 max_len,
                                  const char *name);

    //! Create build dir
    //! \param config Config API
    //! \param app Application API
    void (*compiler_create_build_dir)(struct ConfigApiV0 config,
                                      struct ApplicationApiV0 app);

    //! Get core dir
    //! \return Core dir
    const char *(*compiler_get_core_dir)();

    //! Get source dir
    //! \return Source dir
    const char *(*compiler_get_source_dir)();
};

//! Package API V!
struct PackageApiV0 {
    //! Load package
    //! \param name Package name
    void (*load)(stringid64_t name);

    //! Unload package
    //! \param name Package name
    void (*unload)(stringid64_t name);

    //! Is package loaded
    //! \param name Package name
    int (*is_loaded)(stringid64_t name);

    //! Wait while not package loaded.
    //! \param name Package name
    void (*flush)(stringid64_t name);
};

#endif //CETECH_RESOURCE_TYPES_H
//! |}