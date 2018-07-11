#ifndef CETECH_TEXTURE_H
#define CETECH_TEXTURE_H

#include <cetech/renderer/renderer.h>

#define TEXTURE_TYPE \
    CT_ID64_0("texture", 0xcd4238c6a0c69e32ULL)

#define TEXTURE_INPUT \
    CT_ID64_0("input", 0x2bbcabe5074ade9eULL)

#define TEXTURE_GEN_MIPMAPS \
    CT_ID64_0("gen_mipmaps", 0x22783cd1918e8655ULL)

#define TEXTURE_IS_NORMALMAP \
    CT_ID64_0("is_normalmap", 0xb9378d82634094d1ULL)

#define TEXTURE_DATA \
    CT_ID64_0("texture_data", 0x91074c9e531b0860ULL)

#define TEXTURE_HANDLER_PROP \
    CT_ID64_0("texture_handler", 0x4702d2246fb28a09ULL)



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Api
//==============================================================================

//! Texture API V0
struct ct_texture_a0 {
    struct ct_render_texture_handle (*get)(uint32_t name);
};

CT_MODULE(ct_texture_a0);

#endif //CETECH_TEXTURE_H