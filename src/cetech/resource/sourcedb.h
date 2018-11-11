#ifndef CETECH_SOURCEDB_H
#define CETECH_SOURCEDB_H

#include <stdint.h>
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

struct ct_sourcedb_asset_i0 {
    void (*anotate)(uint64_t obj);
    void (*changed)(uint64_t asset_obj,
                    uint64_t obj,
                    const uint64_t *prop,
                    uint32_t prop_count);
    void (*removed)(uint64_t asset_obj,
                    uint64_t obj,
                    const uint64_t *prop,
                    uint32_t prop_count);
};

struct ct_sourcedb_a0 {
    uint64_t (*get)(struct ct_resource_id resource_id);
    bool (*save)(struct ct_resource_id resource_id);
};

CE_MODULE(ct_sourcedb_a0);

#endif //CETECH_SOURCEDB_H
