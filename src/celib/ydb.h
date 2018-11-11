#ifndef CE_YDB_H
#define CE_YDB_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

struct ce_vio;
struct ce_alloc;

struct ce_ydb_a0 {
    uint64_t (*get_obj)(const char *path);


    void (*save)(const char *path);

    uint64_t (*cdb_from_vio)(struct ce_vio *vio,
                             struct ce_alloc *alloc);

    const char *(*get_key)(uint64_t hash);

    uint64_t (*key)(const char *key);
};

CE_MODULE(ce_ydb_a0);

#endif //CE_YDB_H
