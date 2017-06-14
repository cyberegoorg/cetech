//! \defgroup Resource
//! Resource system
//! \{
#ifndef CETECH_RESOURCE_H
#define CETECH_RESOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

struct vio;
struct allocator;
struct compilator_api;

struct config_api_v0;
struct app_api_v0;

//==============================================================================
// Typedefs
//==============================================================================

//! Resource loader
//! \param input Input vio
//! \param allocator Allocator
//! \return Resource data
typedef void *(*resource_loader_t)(struct vio *input,
                                   struct allocator *allocator);

//! Resource online callback
//! \param name Resource name
//! \param data Resource data
typedef void  (*resource_online_t)(uint64_t name,
                                   void *data);

//! Resource offline callback
//! \param name Resource name
//! \param data Resource data
typedef void  (*resource_offline_t)(uint64_t name,
                                    void *data);

//! Resource unload callback
//! \param new_data Resource data
//! \param allocator Allocator
typedef void  (*resource_unloader_t)(void *new_data,
                                     struct allocator *allocator);

//! Resource reloader
//! \param name Resource name
//! \param old_data Resource old data
//! \param new_data Resource new data
//! \param allocator Allocator
typedef void *(*resource_reloader_t)(uint64_t name,
                                     void *old_data,
                                     void *new_data,
                                     struct allocator *allocator);


struct compilator_api;

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
    const char *source_dir;
    const char *build_dir;
    const char *tmp_dir;

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
struct resource_api_v0 {
    //! Enable autoload feature
    //! \param enable Enable
    void (*set_autoload)(int enable);

    //! Register resource type
    //! \param type Type
    //! \param callbacks Callbacks
    void (*register_type)(uint64_t type,
                          resource_callbacks_t callbacks);

    //! Load resources
    //! \param loaded_data Loaded data array
    //! \param type Type
    //! \param names Names
    //! \param count Names count
    //! \param force Force load
    void (*load)(void **loaded_data,
                 uint64_t type,
                 uint64_t *names,
                 size_t count,
                 int force);

    //! Add loaded resourcess
    //! \param type Types
    //! \param names Names
    //! \param resource_data Resource data array
    //! \param count Resouce count
    void (*add_loaded)(uint64_t type,
                       uint64_t *names,
                       void **resource_data,
                       size_t count);


    //! Load resource (now)
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    void (*load_now)(uint64_t type,
                     uint64_t *names,
                     size_t count);

    //! Unload resources
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    void (*unload)(uint64_t type,
                   uint64_t *names,
                   size_t count);

    //! Reload resources
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    void (*reload)(uint64_t type,
                   uint64_t *names,
                   size_t count);

    //! Reload all loaded resource
    void (*reload_all)();

    //! Is resorce loaded?
    //! \param type Type
    //! \param names Name
    //! \return 1 if is loaded else 0
    int (*can_get)(uint64_t type,
                   uint64_t names);

    //! Is all resorces loaded?
    //! \param type Type
    //! \param names Names
    //! \param count Resource count
    //! \return 1 if is loaded else 0
    int (*can_get_all)(uint64_t type,
                       uint64_t *names,
                       size_t count);

    //! Get resouce data
    //! \param type Type
    //! \param names Name
    //! \return Resource data or NULL if resource is not loaded
    void *(*get)(uint64_t type,
                 uint64_t names);

    //! Type, Name => string
    //! \param str Result string
    //! \param max_len Max stirng len
    //! \param type Type
    //! \param name Name
    //! \return 1 if ok else 0
    int (*type_name_string)(char *str,
                            size_t max_len,
                            uint64_t type,
                            uint64_t name);

#ifdef CETECH_CAN_COMPILE

    //! Register resource compiler
    //! \param type Type
    //! \param compilator Compilator fce
    void (*compiler_register)(uint64_t type,
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
                                 uint64_t type,
                                 uint64_t name);


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
                                  uint32_t max_len,
                                  const char *name);

    //! Create build dir
    //! \param config Config API
    //! \param app Application API
    void (*compiler_create_build_dir)(struct config_api_v0 config,
                                      struct app_api_v0 app);


    //! Get source dir
    //! \return Source dir
    const char *(*compiler_get_source_dir)();

    //! Get core dir
    //! \return Core dir
    const char *(*compiler_get_core_dir)();

#endif

    //! Get build dir
    //! \param build_dir Build dir
    //! \param max_len Max build dir len
    //! \param platform Platform
    //! \return 1 if ok else 0
    int (*compiler_get_build_dir)(char *build_dir,
                                  size_t max_len,
                                  const char *platform);
};


#ifdef __cplusplus
}
#endif

#endif //CETECH_RESOURCE_H
//! |}