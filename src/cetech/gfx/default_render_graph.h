#ifndef CETECH_DEFAULT_RENDER_GRAPH_H
#define CETECH_DEFAULT_RENDER_GRAPH_H


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct ct_render_graph_module;
struct ct_world;

//==============================================================================
// Api (temporary solution)
//==============================================================================

struct ct_default_rg_a0 {
    struct ct_render_graph_module *(*create)(struct ct_world world);
};

CE_MODULE(ct_default_rg_a0);

#endif //CETECH_DEFAULT_RENDER_GRAPH_H
