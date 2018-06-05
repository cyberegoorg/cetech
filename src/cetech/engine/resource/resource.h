//! \defgroup Resource
//! Resource system
//! \{
#ifndef CETECH_RESOURCE_H
#define CETECH_RESOURCE_H

#include <cetech/kernel/yaml/yng.h>
#include <cetech/kernel/cdb/cdb.h>

#define PROP_RESOURECE_DATA CT_ID64_0("data")



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

struct ct_vio;
struct ct_alloc;
struct ct_config_a0;
struct ct_app_a0;
struct ct_compilator_api;

//==============================================================================
// Typedefs
//==============================================================================

//! Resource compilator fce
//! \param filename Source filename
//! \param source_vio Source vio
//! \param build_vio Build vio
//! \param compilator_api Compilator api
typedef void (*ct_resource_compilator_t)(
        const char *filename,
        char **output,
        struct ct_compilator_api *compilator_api);


//==============================================================================
// Structs
//==============================================================================

struct ct_resource_id {
    union {
        struct {
            uint32_t name;
            uint32_t type;
        };
        uint64_t i64;
    };
};

//! Resource callbacks
typedef struct {
    //! Resource online callback
    //! \param name Resource name
    //! \param data Resource data
    void (*online)(uint64_t name,
                   struct ct_vio* input,
                   struct ct_cdb_obj_t *obj);

    //! Resource offline callback
    //! \param name Resource name
    //! \param data Resource data
    void (*offline)(uint64_t name,
                    struct ct_cdb_obj_t *obj);
} ct_resource_type_t;


//! Compilator api
struct ct_compilator_api {
    //! Add dependency
    //! \param who_filname Source filename
    //! \param depend_on_filename Depend on this file
    void (*add_dependency)(const char *who_filname,
                           const char *depend_on_filename);
};


//==============================================================================
// Api
//==============================================================================


struct ct_resource_a0 {
    void (*set_autoload)(bool enable);

    void (*register_type)(const char* type,
                          ct_resource_type_t callbacks);


    void (*load)(uint32_t type,
                 uint32_t *names,
                 size_t count,
                 int force);


    void (*load_now)(uint32_t type,
                     uint32_t *names,
                     size_t count);

    void (*unload)(uint32_t type,
                   uint32_t *names,
                   size_t count);

    void (*reload)(uint32_t type,
                   uint32_t *names,
                   size_t count);


    void (*reload_all)();


    int (*can_get)(uint32_t type,
                   uint32_t names);

    int (*can_get_all)(uint32_t type,
                       uint32_t *names,
                       size_t count);

    struct ct_cdb_obj_t *(*get_obj)(struct ct_resource_id resource_id);

    int (*type_name_string)(char *str,
                            size_t max_len,
                            struct ct_resource_id resourceid);

    void (*compiler_register)(const char* type,
                              ct_resource_compilator_t compilator,
                              bool yaml_based);

    void (*compiler_compile_all)();

    void (*compile_and_reload)(const char *filename);

    int (*compiler_get_filename)(char *filename,
                                 size_t max_ken,
                                 struct ct_resource_id resourceid);

    char *(*compiler_get_tmp_dir)(struct ct_alloc *a,
                                  const char *platform);

    char *(*compiler_external_join)(struct ct_alloc *a,
                                    const char *name);

    void (*compiler_create_build_dir)(struct ct_config_a0 config);

    const char *(*compiler_get_source_dir)();

    const char *(*compiler_get_core_dir)();

    void (*type_name_from_filename)(const char *fullname,
                                    struct ct_resource_id* resource_id,
                                    char *short_name);

    char *(*compiler_get_build_dir)(struct ct_alloc *a,
                                    const char *platform);

};


#endif //CETECH_RESOURCE_H
//! |}