#ifndef CETECH_SOURCEDB_H
#define CETECH_SOURCEDB_H

#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define SOURCEDB_EBUS \
    CE_ID64_0("sourcedb", 0xb1b2935b81b217e8ULL)

#define SOURCEDB_I \
    CE_ID64_0("ct_sourcedb_asset_i0", 0x78a6425ce161b913ULL)

#define RESOURCE_NAME \
    CE_ID64_0("asset_name", 0xf82d0a5475e3d5eaULL)

#define PREFAB_NAME_PROP \
    CE_ID64_0("PREFAB", 0xde35cfdab4ef591dULL)

struct ct_resource_id;
struct ce_alloc;

struct ct_sourcedb_asset_i0 {
    uint64_t (*load)(uint64_t obj);
};

struct ct_sourcedb_a0 {
    uint64_t (*get)(struct ct_resource_id resource_id);
};

CE_MODULE(ct_sourcedb_a0);

#endif //CETECH_SOURCEDB_H
