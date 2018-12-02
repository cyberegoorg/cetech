#ifndef CETECH_SOURCEDB_H
#define CETECH_SOURCEDB_H

#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define SOURCEDB_EBUS \
    CE_ID64_0("sourcedb", 0xb1b2935b81b217e8ULL)

#define SOURCEDB_CHANGED \
    CE_ID64_0("sourcedb_changed", 0xe9ec4483a7a1a19eULL)

#define SOURCEDB_I \
    CE_ID64_0("ct_sourcedb_asset_i0", 0x78a6425ce161b913ULL)

#define ASSET_NAME \
    CE_ID64_0("asset_name", 0xf82d0a5475e3d5eaULL)

#define ASSET_OBJ \
    CE_ID64_0("asset_obj", 0x755d033f781be94ULL)

#define ASSET_CHANGED_OBJ \
    CE_ID64_0("asset_changed_obj", 0xd9e01d669ee07ce4ULL)

#define ASSET_CHANGED_PROP \
    CE_ID64_0("asset_changed_prop", 0x30a907b464c2f909ULL)

#define PREFAB_NAME_PROP \
    CE_ID64_0("PREFAB", 0xde35cfdab4ef591dULL)

struct ct_resource_id;
struct ce_alloc;

struct ct_sourcedb_asset_i0 {
    void (*anotate)(uint64_t obj);

};

struct ct_sourcedb_a0 {
    uint64_t (*get)(struct ct_resource_id resource_id);

    bool (*save)(struct ct_resource_id resource_id);

    bool (*save_all)();

//    void (*set_str)(struct ct_resource_id rid,
//                    uint64_t prop,
//                    uint64_t *keys,
//                    uint64_t keys_n,
//                    const char *value,
//                    const char *new_value);
//
//    void (*set_float)(struct ct_resource_id rid,
//                      uint64_t prop,
//                      uint64_t *keys,
//                      uint64_t keys_n,
//                      float value,
//                      float new_value);
//
//    void (*set_bool)(struct ct_resource_id rid,
//                     uint64_t prop,
//                     uint64_t *keys,
//                     uint64_t keys_n,
//                     bool value,
//                     bool new_value);
//
//    void (*add_subobj)(struct ct_resource_id rid,
//                       uint64_t prop,
//                       uint64_t *keys,
//                       uint64_t keys_n,
//                       uint64_t value,
//                       uint64_t new_value);
//
//    void (*remove_prop)(struct ct_resource_id rid,
//                        uint64_t *keys,
//                        uint64_t keys_n,
//                        uint64_t prop);
};

CE_MODULE(ct_sourcedb_a0);

#endif //CETECH_SOURCEDB_H
