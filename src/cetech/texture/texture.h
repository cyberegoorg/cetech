#ifndef CETECH_TEXTURE_H
#define CETECH_TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#define CT_TEXTURE_A0_STR "ct_texture_a0"

#define CT_TEXTURE_API \
    CE_ID64_0("ct_texture_a0", 0x642bfe2717995f2fULL)

#define CT_TEXTURE_IMPORT_GROUP \
    CE_ID64_0("ct_texture_import_group", 0x1ec6c0c97e3983deULL)

#define CT_DCC_ASSET_TEXTURE_TYPE \
    CE_ID64_0("ct_dcc_asset_texture", 0xccf89315b1a80908ULL)

#define TEXTURE_TASK \
    CE_ID64_0("texture_task", 0xcfe0d7ae816cf881ULL)

#define TEXTURE_TYPE \
    CE_ID64_0("texture", 0xcd4238c6a0c69e32ULL)

#define TEXTURE_INPUT \
    CE_ID64_0("input", 0x2bbcabe5074ade9eULL)

#define TEXTURE_GEN_MIPMAPS \
    CE_ID64_0("gen_mipmaps", 0x22783cd1918e8655ULL)

#define TEXTURE_IS_NORMALMAP \
    CE_ID64_0("is_normalmap", 0xb9378d82634094d1ULL)

#define TEXTURE_DATA \
    CE_ID64_0("texture_data", 0x91074c9e531b0860ULL)

#define TEXTURE_HANDLER_PROP \
    CE_ID64_0("texture_handler", 0x4702d2246fb28a09ULL)


#include <stdint.h>

typedef struct bgfx_texture_handle_s bgfx_texture_handle_t;

//! Texture API V0
struct ct_texture_a0 {
    bgfx_texture_handle_t (*get)(uint64_t name);
};

CE_MODULE(ct_texture_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_TEXTURE_H