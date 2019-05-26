#ifndef CE_YDB_H
#define CE_YDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CE_YDB_API \
    CE_ID64_0("ce_ydb_a0", 0xf5be4873d4ac8920ULL)

typedef struct ce_vio_t0 ce_vio_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_cdb_t0 ce_cdb_t0;
typedef struct cnode_t cnode_t;


struct ce_ydb_a0 {
    uint64_t (*get_obj)(const char *path);

    void (*read_cnodes)(const char *path,
                        cnode_t **cnodes);


    void (*create_root_obj)(cnode_t *cnodes,
                            ce_cdb_t0 tmp_db);

    void (*dump_cnodes)(ce_cdb_t0 db,
                        cnode_t *cnodes,
                        char **outputs);

    uint64_t (*cnodes_from_vio)(ce_vio_t0 *vio,
                                cnode_t **nodes,
                                ce_alloc_t0 *alloc);
};

CE_MODULE(ce_ydb_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_YDB_H
