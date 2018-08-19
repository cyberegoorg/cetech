#ifndef CETECH_RENDER_GRAPH_H
#define CETECH_RENDER_GRAPH_H

#include <stdint.h>


#define RG_OUTPUT_TEXTURE \
    CE_ID64_0("output", 0x759d549788b7b7e0ULL)

#define RENDER_GRAPH_COMPONENT \
    CE_ID64_0("render_graph", 0)


struct ct_render_graph_builder;

struct ct_render_graph_pass {
    uint64_t size;

    void (*on_setup)(void *inst,
                     struct ct_render_graph_builder *builder);

    void (*on_pass)(void *inst,
                    uint8_t viewid,
                    uint64_t layer,
                    struct ct_render_graph_builder *builder);
};

struct ct_render_graph_attachment {
    enum ct_render_backbuffer_ratio ratio;
    enum ct_render_texture_format format;
};

struct ct_render_graph_builder_fce {
    void (*add_pass)(void *inst,
                     struct ct_render_graph_pass *pass,
                     uint64_t layer);

    void (*create)(void *inst,
                   uint64_t name,
                   struct ct_render_graph_attachment info);


    void (*read)(void *inst,
                 uint64_t name);


    struct ct_render_texture_handle (*get_texture)(void *inst,
                                                   uint64_t name);

    void (*set_size)(void *inst,
                     uint16_t w,
                     uint16_t h);

    void (*get_size)(void *inst,
                     uint16_t *size);

    void (*clear)(void *inst);

    void (*execute)(void *inst);
};

struct ct_render_graph_builder {
    void *inst;
    struct ct_render_graph_builder_fce *call;
};

struct ct_render_graph_module_fce {
    void (*add_pass)(void *inst,
                     void *pass,
                     uint64_t size);

    void (*on_setup)(void *inst,
                     struct ct_render_graph_builder *builder);
};

struct ct_render_graph_module {
    void *inst;
    struct ct_render_graph_module_fce *call;
};

struct ct_render_graph_fce {
    void (*add_module)(void *inst,
                       struct ct_render_graph_module *module);

    void (*setup)(void *inst,
                  struct ct_render_graph_builder *builder);
};

struct ct_render_graph {
    void *inst;
    struct ct_render_graph_fce *call;
};

struct ct_render_graph_component {
    struct ct_render_graph *graph;
    struct ct_render_graph_builder* builder;
};

struct ct_render_graph_a0 {
    struct ct_render_graph *(*create_graph)();

    void (*destroy_graph)(struct ct_render_graph *render_graph);

    struct ct_render_graph_module *(*create_module)();

    void (*destroy_module)(struct ct_render_graph_module *module);

    struct ct_render_graph_builder *(*create_builder)();

    void (*destroy_builder)(struct ct_render_graph_builder *builder);
};

CE_MODULE(ct_render_graph_a0);


#endif //CETECH_RENDER_GRAPH_H
