//==============================================================================
// includes
//==============================================================================

#include <cetech/kernel/memory/allocator.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/kernel/containers/array.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/kernel/containers/hash.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/engine/debugdraw/debugdraw.h>


#include "cetech/engine/render_graph/render_graph.h"

CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_dd_a0);

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


static void on_render(uint32_t bus_name,
                      void *event) {
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

static void _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);
    _G = (struct _G) {
            .alloc = ct_memory_a0.main_allocator(),
    };


    ct_ebus_a0.connect(RENDERER_EBUS, RENDERER_RENDER_EVENT, on_render, UINT32_MAX);

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