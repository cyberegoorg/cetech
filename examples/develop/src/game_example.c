#define CE_DYNAMIC_MODULE 1


#include <celib/macros.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/api_system.h>
#include <celib/hashlib.h>
#include <cetech/controlers/keyboard.h>
#include <celib/ebus.h>
#include <celib/cdb.h>
#include <celib/fmath.inl>

#include <cetech/ecs/ecs.h>
#include <cetech/debugui/debugui.h>
#include <cetech/renderer/renderer.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/render_graph/default_render_graph.h>
#include <cetech/transform/transform.h>
#include <cetech/texture/texture.h>
#include <cetech/camera/camera.h>
#include <cetech/kernel/kernel.h>
#include <cetech/controlers/controlers.h>
#include <cetech/game/game_system.h>

static struct G {
    struct ct_world world;
    struct ct_entity render_ent;
    struct ct_entity camera_ent;
    struct ct_entity level;
} _G;

#define _CAMERA_ASSET \
    CE_ID64_0("content/camera", 0x2d0dc3c05bc23f4fULL)

#define _LEVEL_ASSET \
    CE_ID64_0("content/level2", 0x5da9fa42e78626dcULL)


#include "rotation.inl"


void init() {
    _G.world = ct_ecs_a0->create_world();


    _G.camera_ent = ct_ecs_a0->spawn(_G.world, _CAMERA_ASSET);
    _G.level = ct_ecs_a0->spawn(_G.world, _LEVEL_ASSET);


    struct ct_render_graph_builder *builder = ct_render_graph_a0->create_builder();
    struct ct_render_graph *graph = ct_render_graph_a0->create_graph();

    uint64_t rgc = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                            RENDER_GRAPH_COMPONENT);


    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(rgc);
    ce_cdb_a0->set_ptr(w, PROP_RENDER_GRAPH_BUILDER, builder);
    ce_cdb_a0->set_ptr(w, PROP_RENDER_GRAPH_GRAPH, graph);
    ce_cdb_a0->write_commit(w);

    ct_ecs_a0->create(_G.world, &_G.render_ent, 1);
    ct_ecs_a0->add(_G.world, _G.render_ent,
                   (uint64_t[]) {RENDER_GRAPH_COMPONENT}, 1,
                   (uint64_t[]) {rgc});

    struct ct_render_graph_module *module = ct_default_rg_a0->create(_G.world);
    graph->call->add_module(graph, module);
}


void shutdown() {
    ct_ecs_a0->destroy_world(_G.world);
}

void update(float dt) {
    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);

    if (keyboard->button_state(0, keyboard->button_index("v"))) {
        ce_log_a0->info("example", "PO");
        ce_log_a0->error("example", "LICE");
    }

    ct_ecs_a0->simulate(_G.world, dt);
}

static uint64_t game_name() {
    return ce_id_a0->id64("default");
}

static struct ct_render_graph_builder *render_graph_builder() {
    uint64_t rg_comp = ct_ecs_a0->get_one(_G.world, RENDER_GRAPH_COMPONENT,
                                          _G.render_ent);

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(rg_comp);

    return ce_cdb_a0->read_ptr(reader, PROP_RENDER_GRAPH_BUILDER, NULL);
}

struct ct_game_i0 game_i0 = {
        .init = init,
        .shutdown = shutdown,
        .update = update,
        .name = game_name,
        .render_graph_builder = render_graph_builder
};

//==============================================================================
// Module def
//==============================================================================

void CE_MODULE_INITAPI(example_develop)(struct ce_api_a0 *api) {
    if (CE_DYNAMIC_MODULE) {
        CE_INIT_API(api, ct_controlers_a0);
        CE_INIT_API(api, ce_log_a0);
        CE_INIT_API(api, ce_id_a0);
        CE_INIT_API(api, ct_renderer_a0);
        CE_INIT_API(api, ce_ebus_a0);
        CE_INIT_API(api, ct_ecs_a0);
        CE_INIT_API(api, ct_render_graph_a0);
        CE_INIT_API(api, ct_default_rg_a0);
        CE_INIT_API(api, ce_cdb_a0);
        CE_INIT_API(api, ct_game_system_a0);
        CE_INIT_API(api, ce_ydb_a0);
        CE_INIT_API(api, ct_resource_ui_a0);
    }
}

void CE_MODULE_LOAD (example_develop)(struct ce_api_a0 *api,
                                      int reload) {
    CE_UNUSED(api);

    ce_log_a0->info("example", "Init %d", reload);

    api->register_api(GAME_INTERFACE_NAME, &game_i0);

    api->register_api(COMPONENT_INTERFACE_NAME, &rotation_component_i);

    api->register_api(SIMULATION_INTERFACE_NAME, &rotation_simulation_i0);
}

void CE_MODULE_UNLOAD (example_develop)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(api);

    ce_log_a0->info("example", "Shutdown %d", reload);
}

