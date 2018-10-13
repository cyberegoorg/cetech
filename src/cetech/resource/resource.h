#ifndef CETECH_RESOURCE_H
#define CETECH_RESOURCE_H


//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define RESOURCE_TYPE_PROP \
     CE_ID64_0("resource_type", 0x3ebcb3141c07b30ULL)

#define RESOURCE_NAME_PROP \
     CE_ID64_0("resource_name", 0x9137fb038470c85fULL)

#define CONFIG_BUILD \
     CE_ID64_0("build", 0x4429661936ece1eaULL)

#define CONFIG_SRC \
     CE_ID64_0("src", 0x1cdb3620898c588eULL)

#define CONFIG_CORE \
     CE_ID64_0("core", 0x6da99857e9315560ULL)

#define CONFIG_MODULE_DIR \
     CE_ID64_0("module_dir", 0xa96daa49986032f4ULL)

#define CONFIG_EXTERNAL \
     CE_ID64_0("external", 0x9fb8bb487a62dc4fULL)

#define RESOURCE_I_NAME \
    "ct_resource_i0"

#define PROP_RESOURECE_DATA \
    CE_ID64_0("data", 0x8fd0d44d20650b68ULL)

#define RESOURCE_I \
    CE_ID64_0("ct_resource_i0", 0x3e0127963a0db5b9ULL)



struct ce_vio;
struct ce_alloc;
struct ce_config_a0;

//==============================================================================
// Structs
//==============================================================================

struct ct_resource_id {
    union {
        struct {
            uint64_t name;
            uint64_t type;
        };
        uint64_t i128[2];
    };
};

//! Resource callbacks
struct ct_resource_i0 {
    uint64_t (*cdb_type)();

    void *(*get_interface)(uint64_t name_hash);

    void (*online)(uint64_t name,
                   uint64_t obj);

    void (*offline)(uint64_t name,
                    uint64_t obj);

    bool (*compilator)(const char *filename,
                       uint64_t type_keys,
                       struct ct_resource_id rid,
                       const char *fullname);
};

//==============================================================================
// Typedefs
//==============================================================================

typedef bool (*ct_resource_compilator_t)(const char *filename,
                                         uint64_t key,
                                         struct ct_resource_id rid,
                                         const char *fullname);


//==============================================================================
// Api
//==============================================================================


struct ct_resource_a0 {
    struct ct_resource_i0 *(*get_interface)(uint64_t type);

    void (*set_autoload)(bool enable);

    void (*load)(uint64_t type,
                 uint64_t *names,
                 size_t count,
                 int force);

    void (*load_now)(uint64_t type,
                     uint64_t *names,
                     size_t count);

    void (*unload)(uint64_t type,
                   uint64_t *names,
                   size_t count);

    void (*reload)(uint64_t type,
                   uint64_t *names,
                   size_t count);


    void (*reload_all)();


    int (*can_get)(uint64_t type,
                   uint64_t names);

    int (*can_get_all)(uint64_t type,
                       uint64_t *names,
                       size_t count);

    uint64_t (*get)(struct ct_resource_id resource_id);

    int (*type_name_string)(char *str,
                            size_t max_len,
                            struct ct_resource_id resourceid);

    void (*compiler_compile_all)();

    void (*compile_and_reload)(const char *filename);

    int (*compiler_get_filename)(char *filename,
                                 size_t max_ken,
                                 struct ct_resource_id resourceid);

    char *(*compiler_get_tmp_dir)(struct ce_alloc *a,
                                  const char *platform);

    char *(*compiler_external_join)(struct ce_alloc *a,
                                    const char *name);

    bool (*type_name_from_filename)(const char *fullname,
                                    struct ct_resource_id *resource_id,
                                    char *short_name);


};

CE_MODULE(ct_resource_a0);

#endif //CETECH_RESOURCE_H
