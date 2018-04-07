#ifndef CETECH_TEXTURE_H
#define CETECH_TEXTURE_H

#include <cetech/engine/renderer/renderer.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif //CETECH_TEXTURE_H