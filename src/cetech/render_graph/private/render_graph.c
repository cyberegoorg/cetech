//==============================================================================
// includes
//==============================================================================

#include <corelib/allocator.h>
#include <corelib/api_system.h>
#include <corelib/memory.h>
#include <corelib/module.h>
#include <corelib/hashlib.h>
#include <corelib/array.inl>
#include <cetech/renderer/renderer.h>
#include <corelib/hash.inl>
#include <cetech/renderer/renderer.h>
#include <corelib/ebus.h>
#include <cetech/debugdraw/debugdraw.h>


#include "cetech/render_graph/render_graph.h"


#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct render_graph_inst *render_graph_pool;
    struct render_graph_module_inst *render_graph_module_pool;
    struct render_graph_builder_inst *render_graph_builder_pool;

    uint8_t viewid;

    struct ct_alloc *alloc;
} _G;

#include "graph.h"
#include "module.h"
#include "builder.h"


static void on_render(uint64_t event) {
    CT_UNUSED(event);

    _G.viewid = 0;
}

static struct ct_render_graph_a0 render_graph_api = {
        .create_graph = create_render_graph,
        .destroy_graph = destroy_render_graph,
        .create_module = create_module,
        .destroy_module = destroy_module,
        .create_builder = create_render_builder,
        .destroy_builder = destroy_render_builder,
};

struct ct_render_graph_a0 *ct_render_graph_a0 = &render_graph_api;

static void _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);
    _G = (struct _G) {
            .alloc = ct_memory_a0->system,
    };


    ct_ebus_a0->connect(RENDERER_EBUS, RENDERER_RENDER_EVENT, on_render,
                        UINT32_MAX);

    api->register_api("ct_render_graph_a0", &render_graph_api);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        render_graph,
        {
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_dd_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();
        }
)