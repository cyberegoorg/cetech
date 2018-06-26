#ifndef CETECH_RENDER_GRAPH_H
#define CETECH_RENDER_GRAPH_H



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Typedefs
//==============================================================================
struct ct_render_graph_builder;


//==============================================================================
// Pass
//==============================================================================

struct ct_render_graph_pass {
    uint64_t size;

    void (*on_setup)(void *inst,
                     struct ct_render_graph_builder *builder);

    void (*on_pass)(void *inst,
                    uint8_t viewid,
                    uint64_t layer,
                    struct ct_render_graph_builder *builder);
};

//==============================================================================
// Builder
//==============================================================================

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

//==============================================================================
// Module
//==============================================================================

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

//==============================================================================
// Graph
//==============================================================================


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


//==============================================================================
// Api
//==============================================================================


struct ct_render_graph_a0 {
    struct ct_render_graph *(*create_graph)();

    void (*destroy_graph)(struct ct_render_graph *render_graph);

    struct ct_render_graph_module *(*create_module)();

    void (*destroy_module)(struct ct_render_graph_module *module);

    struct ct_render_graph_builder *(*create_builder)();

    void (*destroy_builder)(struct ct_render_graph_builder *builder);
};

CT_MODULE(ct_render_graph_a0);


#endif //CETECH_RENDER_GRAPH_H
