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
#include <cetech/gfx/debugui.h>
#include <cetech/gfx/renderer.h>
#include <cetech/gfx/render_graph.h>
#include <cetech/gfx/default_render_graph.h>
#include <cetech/transform/transform.h>
#include <cetech/gfx/texture.h>
#include <cetech/camera/camera.h>
#include <cetech/kernel/kernel.h>
#include <cetech/controlers/controlers.h>
#include <cetech/game_system/game_system.h>

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
    _G.world = ct_ecs_a0->entity->create_world();


    _G.camera_ent = ct_ecs_a0->entity->spawn(_G.world, _CAMERA_ASSET);
    _G.level = ct_ecs_a0->entity->spawn(_G.world, _LEVEL_ASSET);

    struct ct_render_graph_component rgc = {
            .builder = ct_render_graph_a0->create_builder(),
            .graph = ct_render_graph_a0->create_graph(),
    };

    ct_ecs_a0->entity->create(_G.world, &_G.render_ent, 1);
    ct_ecs_a0->component->add(_G.world, _G.render_ent,
                              (uint64_t[]) {RENDER_GRAPH_COMPONENT}, 1,
                              (void *[]) {&rgc});

    struct ct_render_graph_module *module = ct_default_rg_a0->create(_G.world);
    rgc.graph->call->add_module(rgc.graph, module);
}


void shutdown() {
    ct_ecs_a0->entity->destroy_world(_G.world);
}

void update(float dt) {
    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);

    if (keyboard->button_state(0, keyboard->button_index("v"))) {
        ce_log_a0->info("example", "PO");
        ce_log_a0->error("example", "LICE");
    }

    ct_ecs_a0->system->simulate(_G.world, dt);
}

static uint64_t game_name() {
    return ce_id_a0->id64("default");
}

static struct ct_render_graph_builder *render_graph_builder() {
    struct ct_render_graph_component *rg_comp;
    rg_comp = ct_ecs_a0->component->get_one(_G.world, RENDER_GRAPH_COMPONENT,
                                            _G.render_ent);

    return rg_comp->builder;
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
        CE_INIT_API(api, ct_editor_ui_a0);
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

