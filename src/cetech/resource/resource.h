#ifndef CETECH_RESOURCE_H
#define CETECH_RESOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CT_RESOURCE_A0_STR "ct_resource_a0"

#define CT_RESOURCE_API \
    CE_ID64_0("ct_resource_a0", 0x60a25ca5a83c2ac2ULL)

#define CT_DCC_RESOURCE \
    CE_ID64_0("ct_dcc_resource", 0x43f2068fca45ee4ULL)

#define CT_DCC_FILENAME_PROP \
    CE_ID64_0("filename", 0xbc32f114d9005504ULL)

#define CT_DCC_FILE_MTIME_PROP \
    CE_ID64_0("file_mtime", 0x6e3d88505b7cd5b7ULL)

#define CT_DCC_RESOURCE_ASSETS_PROP \
    CE_ID64_0("assets", 0x64cae6a8bc006a44ULL)

#define CT_DCC_RESOURCE_UID_PROP \
    CE_ID64_0("uuid", 0x35eab629587df71fULL)

#define CDB_UID_PROP \
     CE_ID64_0("cdb_uuid", 0x958a636d68e82bd7ULL)

#define CDB_OBJSET \
     CE_ID64_0("cdb_objset", 0x2b66a0c3813b3490ULL)

#define CDB_TYPE_PROP \
     CE_ID64_0("cdb_type", 0xfe5986c682be99e0ULL)

#define CONFIG_BUILD \
     CE_ID64_0("build", 0x4429661936ece1eaULL)

#define CONFIG_COMPILE \
     CE_ID64_0("compile", 0x3c797c340e1e5467ULL)

#define CT_RESOURCE_I0_STR "ct_resource_i0"

#define CT_RESOURCE_I0 \
    CE_ID64_0("ct_resource_i0", 0x3e0127963a0db5b9ULL)

typedef struct ce_vio_t0 ce_vio_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_cdb_t0 ce_cdb_t0;
typedef struct ce_cdb_uuid_t0 ce_cdb_uuid_t0;

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

//    ct_resource_compilator_t compilator;
} ct_resource_i0;

struct ct_resource_a0 {
    ct_resource_i0 *(*get_interface)(uint64_t type);

    uint64_t (*cdb_loader)(ce_cdb_t0 db,
                           ce_cdb_uuid_t0 uuid);

    bool (*save)(uint64_t uuid);
};

CE_MODULE(ct_resource_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_RESOURCE_H
