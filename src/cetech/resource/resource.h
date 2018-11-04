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

#define RESOURCE_I_NAME \
    "ct_resource_i0"

#define PROP_RESOURECE_DATA \
    CE_ID64_0("data", 0x8fd0d44d20650b68ULL)

#define RESOURCE_I \
    CE_ID64_0("ct_resource_i0", 0x3e0127963a0db5b9ULL)


struct ce_vio;
struct ce_alloc;


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

    uint64_t (*compilator)(const char *filename,
                           uint64_t obj,
                           struct ct_resource_id rid,
                           const char *fullname);
};

typedef uint64_t (*ct_resource_compilator_t)(const char *filename,
                                             uint64_t obj,
                                             struct ct_resource_id rid,
                                             const char *fullname);


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


    void (*reload_from_obj)(struct ct_resource_id resource_id,
                            uint64_t obj);

    int (*type_name_string)(char *str,
                            size_t max_len,
                            struct ct_resource_id resourceid);

};
CE_MODULE(ct_resource_a0);


#endif //CETECH_RESOURCE_H
