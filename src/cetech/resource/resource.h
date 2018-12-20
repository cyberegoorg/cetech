#ifndef CETECH_RESOURCE_H
#define CETECH_RESOURCE_H


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define ASSET_TYPE_PROP \
     CE_ID64_0("asset_type", 0x1f1f05db4e4dabbaULL)

 #define ASSET_NAME_PROP \
     CE_ID64_0("asset_name", 0xf82d0a5475e3d5eaULL)

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
    uint64_t uid;
};


typedef uint64_t (*ct_resource_compilator_t)(uint64_t obj,
                                             struct ct_resource_id rid,
                                             const char *fullname);


//! Resource interface
struct ct_resource_i0 {
    uint64_t (*cdb_type)();

    void *(*get_interface)(uint64_t name_hash);

    void (*online)(uint64_t name,
                   uint64_t obj);

    void (*offline)(uint64_t name,
                    uint64_t obj);

    ct_resource_compilator_t compilator;
};



struct ct_resource_a0 {
    struct ct_resource_i0 *(*get_interface)(uint64_t type);

    bool (*cdb_loader)(uint64_t uid);
    void (*reload_from_obj)(struct ct_resource_id resource_id, uint64_t obj);
};

CE_MODULE(ct_resource_a0);


#endif //CETECH_RESOURCE_H
