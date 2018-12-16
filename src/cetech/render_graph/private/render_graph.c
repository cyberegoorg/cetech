//==============================================================================
// includes
//==============================================================================

#include <celib/allocator.h>
#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/module.h>
#include <celib/hashlib.h>
#include <celib/array.inl>
#include <celib/hash.inl>
#include <celib/ebus.h>

#include <cetech/renderer/renderer.h>
#include <cetech/debugdraw/debugdraw.h>
#include <cetech/ecs/ecs.h>
#include <cetech/kernel/kernel.h>
#include <cetech/transform/transform.h>
#include <celib/cdb.h>
#include "cetech/render_graph/render_graph.h"


#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct render_graph_inst *render_graph_pool;
    struct render_graph_module_inst *render_graph_module_pool;
    struct render_graph_builder_inst *render_graph_builder_pool;
    struct ce_alloc *alloc;
} _G;

#include "graph.h"
#include "module.h"
#include "builder.h"


static uint64_t cdb_type() {
    return RENDER_GRAPH_COMPONENT;
}

//static uint64_t size() {
//    return sizeof(struct ct_render_graph_component);
//}

static struct ct_component_i0 render_graph_component_i = {
//        .size = size,
        .cdb_type = cdb_type,
};


static void foreach_render_graph(struct ct_world world,
                                 struct ct_entity *ent,
                                 ct_entity_storage_t *item,
                                 uint32_t n,
                                 void *data) {
    for (uint32_t i = 1; i < n; ++i) {
        uint64_t rg = ct_ecs_a0->get_one(world, RENDER_GRAPH_COMPONENT, ent[i]);

        const ce_cdb_obj_o *reader = ce_cdb_a0->read(rg);

        struct ct_render_graph *graph = ce_cdb_a0->read_ptr(reader,
                                                            PROP_RENDER_GRAPH_GRAPH,
                                                            NULL);

        struct ct_render_graph_builder *builder = ce_cdb_a0->read_ptr(reader,
                                                                      PROP_RENDER_GRAPH_BUILDER,
                                                                      NULL);

        uint16_t size[2] = {};
        builder->get_size(builder, size);
        if ((size[0] == 0) || (size[1] == 0)) {
            continue;
        }

        builder->clear(builder);
        graph->setup(graph, builder);
        builder->execute(builder);
    }
}

static void render_system(struct ct_world world,
                          float dt) {
    uint64_t mask = ct_ecs_a0->mask(RENDER_GRAPH_COMPONENT);
    ct_ecs_a0->process(world, mask, foreach_render_graph, &dt);
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


static uint64_t render_system_name() {
    return RENDER_SYSTEM;
}

static const uint64_t *rendersystem_after(uint32_t *n) {
    static uint64_t _after[] = {TRANSFORM_SYSTEM};
    *n = CE_ARRAY_LEN(_after);
    return _after;
}

static struct ct_simulation_i0 render_simulation_i0 = {
        .simulation = render_system,
        .name = render_system_name,
        .after = rendersystem_after,
};

static void _init(struct ce_api_a0 *api) {
    CE_UNUSED(api);
    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };

    api->register_api("ct_render_graph_a0", &render_graph_api);
    api->register_api(COMPONENT_INTERFACE_NAME, &render_graph_component_i);
    api->register_api(SIMULATION_INTERFACE_NAME, &render_simulation_i0);

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