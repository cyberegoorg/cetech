#ifndef CETECH_MATERIAL_H
#define CETECH_MATERIAL_H

#include <stdint.h>
#include <celib/cdb.h>

struct ct_cdb_obj_t;

#define CT_MATERIAL_API \
    CE_ID64_0("ct_material_a0", 0xdfd0a5e43df0846dULL)

#define RENDER_STATE_RGB_WRITE \
    CE_ID64_0("rgb_write", 0xdad21ff8b23271ffULL)

#define RENDER_STATE_ALPHA_WRITE \
    CE_ID64_0("alpha_write", 0x93c0953aa6e40b10ULL)

#define RENDER_STATE_DEPTH_WRITE \
    CE_ID64_0("depth_write", 0x6d5cef63be1e7b46ULL)

#define RENDER_STATE_DEPTH_TEST_LESS \
    CE_ID64_0("depth_test_less", 0x25d531ce0f04418eULL)

#define RENDER_STATE_CULL_CCW \
    CE_ID64_0("cull_ccw", 0x8447d75aa845c612ULL)

#define RENDER_STATE_CULL_CW \
    CE_ID64_0("cull_cw", 0x6b5530bb1cba7b79ULL)

#define RENDER_STATE_MSAA \
    CE_ID64_0("msaa", 0xdc0268c3aab08183ULL)

#define MATERIAL_TYPE \
    CE_ID64_0("material", 0xeac0b497876adedfULL)

#define MATERIAL_SHADER_PROP \
    CE_ID64_0("shader", 0xcce8d5b5f5ae333fULL)

#define MATERIAL_STATE_PROP \
    CE_ID64_0("state", 0x82830aedd03d8beeULL)

#define MATERIAL_VARIABLES_PROP \
    CE_ID64_0("variables", 0x4fb1ab3fd540bd03ULL)

#define MATERIAL_VAR_HANDLER_PROP \
    CE_ID64_0("handler", 0xe08f763ff8f80d07ULL)

#define MATERIAL_VAR_NAME_PROP \
    CE_ID64_0("name", 0xd4c943cba60c270bULL)

#define MATERIAL_VAR_TYPE_PROP \
    CE_ID64_0("type", 0xa21bd0e01ac8f01fULL)

#define MATERIAL_VAR_VALUE_PROP \
    CE_ID64_0("value", 0x920b430f38928dc9ULL)

#define MATERIAL_VAR_VALUE_PROP_X \
    CE_ID64_0("value.x", 0xfa204d422e6d6299ULL)

#define MATERIAL_VAR_VALUE_PROP_Y \
    CE_ID64_0("value.y", 0xe5776873c594fbf5ULL)

#define MATERIAL_VAR_VALUE_PROP_Z \
    CE_ID64_0("value.z", 0x150e7a91a060d411ULL)

#define MATERIAL_VAR_VALUE_PROP_W \
    CE_ID64_0("value.w", 0x7e696a767c2af865ULL)

#define MATERIAL_LAYERS \
    CE_ID64_0("layers", 0x289219f856ee5cd6ULL)

#define MATERIAL_LAYER_NAME \
    CE_ID64_0("name", 0xd4c943cba60c270bULL)

struct ct_material_a0 {
    uint64_t (*create)(uint64_t name);

    void (*submit)(uint64_t material,
                   uint64_t layer,
                   uint8_t viewid);

    void (*set_texture_handler)(uint64_t material,
                                uint64_t layer,
                                const char *slot,
                                struct ct_render_texture_handle texture);
};

CE_MODULE(ct_material_a0);

#endif //CETECH_MATERIAL_H