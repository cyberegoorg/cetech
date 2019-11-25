#ifndef CETECH_RENDER_GRAPH_H
#define CETECH_RENDER_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_RG_A0_STR "ct_rg_a0"

#define CT_RG_API \
    CE_ID64_0("ct_rg_a0", 0xa4e3156b3079009bULL)


#define RG_OUTPUT_TEXTURE \
    CE_ID64_0("output", 0x759d549788b7b7e0ULL)

typedef struct ct_rg_builder_t0 ct_rg_builder_t0;
typedef enum bgfx_backbuffer_ratio bgfx_backbuffer_ratio_t;
typedef enum bgfx_texture_format bgfx_texture_format_t;

typedef struct ct_rg_pass_t0 {
    uint64_t size;

    void (*on_setup)(void *inst,
                     ct_rg_builder_t0 *builder);

    void (*on_pass)(void *inst,
                    uint8_t viewid,
                    uint64_t layer,
                    ct_camera_data_t0 *main_camera,
                    ct_rg_builder_t0 *builder);
} ct_rg_pass_t0;

typedef struct ct_rg_attachment_t0 {
    bgfx_backbuffer_ratio_t ratio;
    bgfx_texture_format_t format;
} ct_rg_attachment_t0;

typedef struct ct_rg_builder_t0 {
    void *inst;

    void (*add_pass)(void *inst,
                     struct ct_rg_pass_t0 *pass,
                     uint64_t layer);

    void (*create)(void *inst,
                   uint64_t name,
                   struct ct_rg_attachment_t0 info);


    void (*read)(void *inst,
                 uint64_t name);

    void (*write)(void *inst,
                  uint64_t name);

    bgfx_texture_handle_t (*get_texture)(void *inst,
                                         uint64_t name);

    void (*set_size)(void *inst,
                     uint16_t w,
                     uint16_t h);

    void (*get_size)(void *inst,
                     uint16_t *size);

    uint8_t (*get_layer_viewid)(void *inst,
                                uint64_t layer_name);

    void (*clear)(void *inst);

    void (*execute)(void *inst,
                    ct_camera_data_t0 *main_camera);
} ct_rg_builder_t0;

typedef struct ct_rg_module_t0 {
    void *inst;

    void (*add_module)(void *inst,
                       struct ct_rg_module_t0 *module);

    void (*add_pass)(void *inst,
                     void *pass,
                     uint64_t size);

    ct_rg_module_t0 *(*add_extension_point)(void *inst,
                                            uint64_t name);

    ct_rg_module_t0 *(*get_extension_point)(void *inst,
                                            uint64_t name);

    void (*on_setup)(void *inst,
                     ct_rg_builder_t0 *builder);
} ct_rg_module_t0;

struct ct_rg_a0 {
    ct_rg_module_t0 *(*create_module)();

    void (*destroy_module)(ct_rg_module_t0 *module);

    ct_rg_builder_t0 *(*create_builder)();
    void (*destroy_builder)(ct_rg_builder_t0 *builder);

    void (*screenspace_quad)(float texture_width,
                             float texture_height,
                             float texel_half,
                             float width,
                             float height);
};

CE_MODULE(ct_rg_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_RENDER_GRAPH_H
