#ifndef CETECH_RESOURCE_H
#define CETECH_RESOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CT_RESOURCE_API \
    CE_ID64_0("ct_resource_a0", 0x60a25ca5a83c2ac2ULL)

#define CDB_UID_PROP \
     CE_ID64_0("cdb_uid", 0x644d24512264abb2ULL)

#define CDB_OBJSET \
     CE_ID64_0("cdb_objset", 0x2b66a0c3813b3490ULL)

#define CDB_TYPE_PROP \
     CE_ID64_0("cdb_type", 0xfe5986c682be99e0ULL)

#define CONFIG_BUILD \
     CE_ID64_0("build", 0x4429661936ece1eaULL)

#define CONFIG_COMPILE \
     CE_ID64_0("compile", 0x3c797c340e1e5467ULL)

#define CT_RESOURCE_I \
    CE_ID64_0("ct_resource_i0", 0x3e0127963a0db5b9ULL)

typedef struct ce_vio_t0 ce_vio_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_cdb_t0 ce_cdb_t0;

typedef struct ct_resource_id_t0 {
    uint64_t uid;
} ct_resource_id_t0;

typedef bool (*ct_resource_compilator_t)(ce_cdb_t0 db,
                                         uint64_t obj);

//! Resource interface
typedef struct ct_resource_i0 {
    const char *(*name)();

    uint64_t (*cdb_type)();

    const char *(*display_icon)();

    void *(*get_interface)(uint64_t name_hash);

    void (*online)(ce_cdb_t0 db,
                   uint64_t obj);

    void (*offline)(ce_cdb_t0 db,
                    uint64_t obj);

    void (*create_new)(uint64_t obj);

    ct_resource_compilator_t compilator;
} ct_resource_i0;


struct ct_resource_a0 {
    ct_resource_i0 *(*get_interface)(uint64_t type);

    bool (*cdb_loader)(ce_cdb_t0 db,
                       uint64_t uid);

    bool (*save)(uint64_t uid);

    bool (*save_to_db)(uint64_t uid);
};

CE_MODULE(ct_resource_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_RESOURCE_H
