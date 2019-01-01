#ifndef CETECH_SHADER_H
#define CETECH_SHADER_H

#include <celib/cdb.h>

#include <stdint.h>
#include <cetech/renderer/renderer.h>

#define CT_SHADER_API \
    CE_ID64_0("ct_shader_a0", 0x6a9d892dacfc7576ULL)


#define SHADER_TYPE \
    CE_ID64_0("shader", 0xcce8d5b5f5ae333fULL)

#define SHADER_VS_INPUT \
    CE_ID64_0("vs_input", 0x2d6d9196e29625fdULL)

#define SHADER_FS_INPUT \
    CE_ID64_0("fs_input", 0x7ed33aaa3bf6f312ULL)

#define SHADER_VS_DATA \
    CE_ID64_0("vs_data", 0xb2eef410e8bf776aULL)

#define SHADER_FS_DATA \
    CE_ID64_0("fs_data", 0xa754c8f87442bf42ULL)

#define SHADER_PROP \
    CE_ID64_0("shader", 0xcce8d5b5f5ae333fULL)

//! Shader API V0
struct ct_shader_a0 {
    ct_render_program_handle_t (*get)(uint64_t shader);
};

CE_MODULE(ct_shader_a0);

#endif //CETECH_SHADER_H