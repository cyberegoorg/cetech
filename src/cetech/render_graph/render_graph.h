#ifndef CETECH_RENDER_GRAPH_H
#define CETECH_RENDER_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_RG_API \
    CE_ID64_0("ct_rg_a0", 0xa4e3156b3079009bULL)


#define RG_OUTPUT_TEXTURE \
    CE_ID64_0("output", 0x759d549788b7b7e0ULL)

struct ct_rg_builder;
typedef enum bgfx_backbuffer_ratio bgfx_backbuffer_ratio_t;
typedef enum bgfx_texture_format bgfx_texture_format_t;

struct ct_rg_pass {
    uint64_t size;

    void (*on_setup)(void *inst,
                     struct ct_rg_builder *builder);

    void (*on_pass)(void *inst,
                    uint8_t viewid,
                    uint64_t layer,
                    struct ct_rg_builder *builder);
};

struct ct_rg_attachment {
    bgfx_backbuffer_ratio_t ratio;
    bgfx_texture_format_t format;
};

struct ct_rg_builder {
    void *inst;

    void (*add_pass)(void *inst,
                     struct ct_rg_pass *pass,
                     uint64_t layer);

    void (*create)(void *inst,
                   uint64_t name,
                   struct ct_rg_attachment info);


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

    uint8_t (*get_layer_viewid)(void *inst, uint64_t layer_name);

    void (*clear)(void *inst);

    void (*execute)(void *inst);
};

struct ct_rg_module {
    void *inst;

    void (*add_module)(void *inst,
                       struct ct_rg_module *module);

    void (*add_pass)(void *inst,
                     void *pass,
                     uint64_t size);

    struct ct_rg_module* (*add_extension_point)(void *inst,
                                uint64_t name);

    struct ct_rg_module *(*get_extension_point)(void *inst,
                                                uint64_t name);

    void (*on_setup)(void *inst,
                     struct ct_rg_builder *builder);
};

struct ct_rg {
    void *inst;

    void (*set_module)(void *inst,
                       struct ct_rg_module *module);

    void (*setup)(void *inst,
                  struct ct_rg_builder *builder);
};

struct ct_rg_a0 {
    struct ct_rg *(*create_graph)();

    void (*destroy_graph)(struct ct_rg *render_graph);

    struct ct_rg_module *(*create_module)();

    void (*destroy_module)(struct ct_rg_module *module);

    struct ct_rg_builder *(*create_builder)();

    void (*destroy_builder)(struct ct_rg_builder *builder);
};

CE_MODULE(ct_rg_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_RENDER_GRAPH_H
