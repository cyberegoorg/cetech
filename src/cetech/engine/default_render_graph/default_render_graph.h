#ifndef CETECH_DEFAULT_RENDER_GRAPH_H
#define CETECH_DEFAULT_RENDER_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct ct_render_graph_module;
struct ct_world;
//==============================================================================
// Api
//==============================================================================

struct ct_default_render_graph_a0 {
    struct ct_render_graph_module *(*create)(struct ct_world world);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_DEFAULT_RENDER_GRAPH_H
