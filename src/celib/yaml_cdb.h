#ifndef CE_YDB_H
#define CE_YDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CE_YDB_A0_STR "ce_yaml_cdb_a0"

#define CE_YDB_API \
    CE_ID64_0("ce_yaml_cdb_a0", 0xe00524e73f978a88ULL)

typedef struct ce_vio_t0 ce_vio_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_cdb_t0 ce_cdb_t0;
typedef struct cnode_t cnode_t;
typedef struct ce_cdb_uuid_t0 ce_cdb_uuid_t0;

struct ce_yaml_cdb_a0 {
    uint64_t (*get_obj)(const char *path);

    void (*dump_str)(ce_cdb_t0 db,
                     char **buffer,
                     uint64_t obj,
                     uint32_t level);

    ce_cdb_uuid_t0 (*cnodes_from_vio)(ce_vio_t0 *vio,
                                      cnode_t **nodes,
                                      ce_alloc_t0 *alloc);
};

CE_MODULE(ce_yaml_cdb_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_YDB_H
