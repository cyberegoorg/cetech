#ifndef CETECH_SHADER_H
#define CETECH_SHADER_H

#include <corelib/cdb.h>



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <cetech/renderer/renderer.h>

#define SHADER_TYPE \
    CT_ID64_0("shader", 0xcce8d5b5f5ae333fULL)

#define SHADER_VS_INPUT \
    CT_ID64_0("vs_input", 0x2d6d9196e29625fdULL)

#define SHADER_FS_INPUT \
    CT_ID64_0("fs_input", 0x7ed33aaa3bf6f312ULL)

#define SHADER_VS_DATA \
    CT_ID64_0("vs_data", 0xb2eef410e8bf776aULL)

#define SHADER_FS_DATA \
    CT_ID64_0("fs_data", 0xa754c8f87442bf42ULL)

#define SHADER_PROP \
    CT_ID64_0("shader", 0xcce8d5b5f5ae333fULL)

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Api
//==============================================================================

//! Shader API V0
struct ct_shader_a0 {
    struct ct_render_shader_handle (*get)(uint64_t shader);
};

CT_MODULE(ct_shader_a0);

#endif //CETECH_SHADER_H