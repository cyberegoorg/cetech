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

struct ce_ydb_a0 {
    uint64_t (*get_obj)(const char *path);


    void (*save)(const char *path);

    uint64_t (*cdb_from_vio)(ce_vio_t0 *vio,
                             ce_alloc_t0 *alloc);

    const char *(*get_key)(uint64_t hash);

    uint64_t (*key)(const char *key);
};

CE_MODULE(ce_ydb_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_YDB_H
