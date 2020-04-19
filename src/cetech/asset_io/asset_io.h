#ifndef CETECH_ASSET_IO_H
#define CETECH_ASSET_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CT_ASSET_IO_A0_STR "ct_asset_io_a0"

#define CT_ASSET_IO_API \
    CE_ID64_0("ct_asset_io_a0", 0x823c8aaed53dfa2aULL)

#define CT_ASSET_IO_I0_STR "ct_asset_io_i0"

#define CT_ASSET_IO_I0 \
    CE_ID64_0("ct_asset_io_i0", 0xf1029647efb47d31ULL)

#define CT_DCC_ASSET_IO_I0_STR "ct_asset_dcc_io_i0"

#define CT_DCC_ASSET_IO_I0 \
    CE_ID64_0("ct_asset_dcc_io_i0", 0xd08dd66b6b1f23c0ULL)

#define CT_DCC_IMPORT_GROUP \
    CE_ID64_0("ct_dcc_import_group", 0xe46eb2671b198931ULL)

#define CT_CORE_IMPORT_GROUP \
    CE_ID64_0("ct_core_import_group", 0xb5f6057b07861723ULL)

#define CONFIG_SRC \
     CE_ID64_0("src", 0x1cdb3620898c588eULL)

#define CONFIG_CORE \
     CE_ID64_0("core", 0x6da99857e9315560ULL)

#define CONFIG_EXTERNAL \
     CE_ID64_0("external", 0x9fb8bb487a62dc4fULL)


typedef struct ct_asset_io_i0 {
    bool (*supported_extension)(const char *extension);

    bool (*import)(ce_cdb_t0 db,
                   uint64_t obj);

} ct_asset_io_i0;

typedef struct ct_asset_dcc_io_i0 {
    uint64_t import_group;

    bool (*supported_extension)(const char *extension);

    bool (*import_dcc)(ce_cdb_t0 db,
                       uint64_t dcc_obj);

} ct_asset_dcc_io_i0;

struct ct_asset_io_a0 {
    ct_asset_io_i0 *(*find_asset_io)(const char *extension);

    ct_asset_dcc_io_i0 *(*find_asset_dcc_io)(const char *extension,
                                             uint64_t group);

    void (*create_import_group)(uint64_t name,
                                float order);


};

CE_MODULE(ct_asset_io_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ASSET_IO_H
