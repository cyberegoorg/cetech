#ifndef CE_YDB_H
#define CE_YDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CE_YDB_A0_STR "ce_cdb_yaml_a0"

#define CE_YDB_API \
    CE_ID64_0("ce_cdb_yaml_a0", 0x6d030b6cbf566224ULL)

typedef struct ce_vio_t0 ce_vio_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_cdb_t0 ce_cdb_t0;
typedef struct ct_cdb_node_t ct_cdb_node_t;
typedef struct ce_cdb_uuid_t0 ce_cdb_uuid_t0;

struct ce_cdb_yaml_a0 {
    uint64_t (*load_from_file)(const char *path);

    bool (*save_to_file)(ce_cdb_t0 db,
                         uint64_t fs_root,
                         const char *path,
                         uint64_t obj);

    ce_cdb_uuid_t0 (*load_to_nodes)(const char *path,
                                    ce_vio_t0 *vio,
                                    ct_cdb_node_t **nodes,
                                    ce_alloc_t0 *alloc);
};

CE_MODULE(ce_cdb_yaml_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_YDB_H
