#ifndef CETECH_ASSET_H
#define CETECH_ASSET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CT_ASSET_A0_STR "ct_asset_a0"

#define CT_ASSET_API \
   CCE_ID64_0("ct_asset_a0", 0x3f2865d0ba1003cULL)

#define CT_DCC_ASSET \
    CE_ID64_0("ct_dcc_asset", 0xed663d9986dbea5eULL)

#define CT_DCC_FILENAME_PROP \
    CE_ID64_0("filename", 0xbc32f114d9005504ULL)

#define CT_DCC_FILE_MTIME_PROP \
    CE_ID64_0("file_mtime", 0x6e3d88505b7cd5b7ULL)

#define CT_DCC_ASSET_ASSETS_PROP \
    CE_ID64_0("assets", 0x64cae6a8bc006a44ULL)

#define CT_DCC_ASSET_UID_PROP \
    CE_ID64_0("uuid", 0x35eab629587df71fULL)

#define CDB_UID_PROP \
     CE_ID64_0("cdb_uuid", 0x958a636d68e82bd7ULL)

#define CDB_OBJSET \
     CE_ID64_0("cdb_objset", 0x2b66a0c3813b3490ULL)

#define CDB_TYPE_PROP \
     CE_ID64_0("cdb_type", 0xfe5986c682be99e0ULL)

#define CONFIG_COMPILE \
     CE_ID64_0("compile", 0x3c797c340e1e5467ULL)

#define CT_ASSET_I0_STR "ct_asset_i0"

#define CT_ASSET_I0 \
    CE_ID64_0("ct_asset_i0", 0x47d885a0efa7f917ULL)

typedef struct ce_vio_t0 ce_vio_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_cdb_t0 ce_cdb_t0;
typedef struct ce_cdb_uuid_t0 ce_cdb_uuid_t0;

//! Asset interface
typedef struct ct_asset_i0 {
    const char *(*name)();

    uint64_t (*cdb_type)();

    const char *(*display_icon)();

    void *(*get_interface)(uint64_t name_hash);

    void (*online)(ce_cdb_t0 db,
                   uint64_t obj);

    void (*offline)(ce_cdb_t0 db,
                    uint64_t obj);

    void (*create_new)(uint64_t obj);
} ct_asset_i0;

struct ct_asset_a0 {
    ct_asset_i0 *(*get_interface)(uint64_t type);

    uint64_t (*cdb_loader)(ce_cdb_t0 db,
                           ce_cdb_uuid_t0 uuid);

    bool (*save)(uint64_t uuid);

    int (*list_assets_from_dirs)(const char *dir,
                                 char ***assets,
                                 ce_alloc_t0 *alloc);

    int (*list_assets_by_type)(const char *name,
                               const char *type,
                               char ***assets,
                               ce_alloc_t0 *alloc);

    void (*clean_assets_list)(char **assets,
                              ce_alloc_t0 *alloc);

    uint64_t (*get_asset_type)(ce_cdb_uuid_t0 asset);

    void (*gen_tmp_file)(char *output,
                         uint64_t max_size,
                         const char *platform,
                         const char *filename,
                         const char *ext);

    char *(*external_join)(ce_alloc_t0 *a,
                           const char *name);

    void (*compile_all)();

    const char *(*asset_filename)(struct ce_cdb_uuid_t0 uid);

    struct ce_cdb_uuid_t0 (*filename_asset)(const char *filename);
    void (*save_to_cdb)(ce_cdb_t0 db,
                        uint64_t obj,
                        const char *filename);

    bool (*save_to_file)(ce_cdb_t0 db,
                         uint64_t obj,
                         const char *filename);
};

CE_MODULE(ct_asset_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ASSET_H
