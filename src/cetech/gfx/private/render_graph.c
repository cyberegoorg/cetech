//==============================================================================
// includes
//==============================================================================

#include <celib/allocator.h>
#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/module.h>
#include <celib/hashlib.h>
#include <celib/array.inl>
#include <cetech/gfx/renderer.h>
#include <celib/hash.inl>
#include <cetech/gfx/renderer.h>
#include <celib/ebus.h>
#include <cetech/gfx/debugdraw.h>
#include <cetech/ecs/ecs.h>
#include <cetech/kernel/kernel.h>


#include "cetech/gfx/render_graph.h"


#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct render_graph_inst *render_graph_pool;
    struct render_graph_module_inst *render_graph_module_pool;
    struct render_graph_builder_inst *render_graph_builder_pool;

    uint8_t viewid;
    struct ce_alloc *alloc;
} _G;

#include "graph.h"
#include "module.h"
#include "builder.h"


static uint64_t cdb_type() {
    return RENDER_GRAPH_COMPONENT;
}

static uint64_t size() {
    return sizeof(struct ct_render_graph_component);
}

static struct ct_component_i0 render_graph_component_i = {
        .size = size,
        .cdb_type = cdb_type,
};


static void foreach_render_graph(struct ct_world world,
                             struct ct_entity *ent,
                             ct_entity_storage_t *item,
                             uint32_t n,
                             void *data) {
    struct ct_render_graph_component *all_rg;
    all_rg = ct_ecs_a0->component->get_all(RENDER_GRAPH_COMPONENT, item);

    for (uint32_t i = 1; i < n; ++i) {
        struct ct_render_graph_component rg = all_rg[i];

        rg.builder->call->clear(rg.builder);
        rg.graph->call->setup(rg.graph, rg.builder);
        rg.builder->call->execute(rg.builder);
    }
}

static void render_system(struct ct_world world,
                            float dt) {
    uint64_t mask = ct_ecs_a0->component->mask(RENDER_GRAPH_COMPONENT);
    ct_ecs_a0->system->process(world, mask, foreach_render_graph, &dt);
}

static void on_render(uint64_t event) {
    CE_UNUSED(event);

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

static void _init(struct ce_api_a0 *api) {
    CE_UNUSED(api);
    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };


    ce_ebus_a0->connect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, on_render,
                        KERNEL_ORDER + 1);

    api->register_api("ct_render_graph_a0", &render_graph_api);
    api->register_api(COMPONENT_INTERFACE_NAME, &render_graph_component_i);

    ct_ecs_a0->system->register_simulation("render", render_system);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        render_graph,
        {
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_renderer_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_dd_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            _shutdown();
        }
)