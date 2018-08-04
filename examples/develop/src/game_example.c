#define CT_DYNAMIC_MODULE 1

#include <corelib/macros.h>

#include <corelib/log.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include <corelib/hashlib.h>

#include <cetech/controlers/keyboard.h>
#include <corelib/ebus.h>

#include <corelib/cdb.h>
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
#include <string.h>
#include <cetech/game_system/game_system.h>
#include <corelib/fmath.inl>

static struct G {
    struct ct_world world;
    struct ct_entity camera_ent;
    struct ct_entity level;

    struct ct_render_graph *render_graph;
    struct ct_render_graph_builder *render_graph_builder;
} _G;

#define _CAMERA_ASSET \
    CT_ID64_0("content/camera", 0x2d0dc3c05bc23f4fULL)

#define _LEVEL_ASSET \
    CT_ID64_0("content/level2", 0x5da9fa42e78626dcULL)

void init() {
    _G.world = ct_ecs_a0->entity->create_world();
    _G.camera_ent = ct_ecs_a0->entity->spawn(_G.world, _CAMERA_ASSET);
    _G.level = ct_ecs_a0->entity->spawn(_G.world, _LEVEL_ASSET);

    _G.render_graph = ct_render_graph_a0->create_graph();
    _G.render_graph_builder = ct_render_graph_a0->create_builder();

    struct ct_render_graph_module *module = ct_default_rg_a0->create(_G.world);
    _G.render_graph->call->add_module(_G.render_graph, module);
}


void shutdown() {
    ct_render_graph_a0->destroy_builder(_G.render_graph_builder);
    ct_render_graph_a0->destroy_graph(_G.render_graph);
    ct_ecs_a0->entity->destroy_world(_G.world);
}

void update(float dt) {
    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);


    if (keyboard->button_state(0, keyboard->button_index("v"))) {
        ct_log_a0->info("example", "PO");
        ct_log_a0->error("example", "LICE");
    }

    struct ct_entity ent;
    ent = ct_ecs_a0->entity->find_by_name(_G.world, _G.level,
                                          ct_hashlib_a0->id64("body"));

    uint64_t ent_obj = ent.h;
    uint64_t components = ct_cdb_a0->read_subobject(ent_obj,
                                                    ENTITY_COMPONENTS, 0);
    uint64_t component = ct_cdb_a0->read_subobject(components,
                                                   TRANSFORM_COMPONENT, 0);

    float rot[3] = {0};
    ct_cdb_a0->read_vec3(component, PROP_ROTATION, rot);

    ct_vec3_add_s(rot, rot, 5.0f * dt);

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(component);
    ct_cdb_a0->set_vec3(w, PROP_ROTATION, rot);
    ct_cdb_a0->write_commit(w);

    ct_ecs_a0->system->simulate(_G.world, dt);
}

static void render() {
    _G.render_graph_builder->call->clear(_G.render_graph_builder);
    _G.render_graph->call->setup(_G.render_graph, _G.render_graph_builder);
    _G.render_graph_builder->call->execute(_G.render_graph_builder);
}

static uint64_t name() {
    return ct_hashlib_a0->id64("default");
}

static struct ct_render_graph_builder *render_graph_builder() {
    return _G.render_graph_builder;
}

struct ct_game_i0 game_i0 = {
        .init = init,
        .shutdown = shutdown,
        .render = render,
        .update = update,
        .name = name,
        .render_graph_builder = render_graph_builder
};

//==============================================================================
// Module def
//==============================================================================

//==============================================================================
// Init api
//==============================================================================
void CETECH_MODULE_INITAPI(example_develop)(struct ct_api_a0 *api) {
    if (CT_DYNAMIC_MODULE) {
        CT_INIT_API(api, ct_controlers_a0);
        CT_INIT_API(api, ct_log_a0);
        CT_INIT_API(api, ct_hashlib_a0);
        CT_INIT_API(api, ct_renderer_a0);
        CT_INIT_API(api, ct_ebus_a0);
        CT_INIT_API(api, ct_ecs_a0);
        CT_INIT_API(api, ct_render_graph_a0);
        CT_INIT_API(api, ct_default_rg_a0);
        CT_INIT_API(api, ct_cdb_a0);
        CT_INIT_API(api, ct_game_system_a0);
    }
}

void CETECH_MODULE_LOAD (example_develop)(struct ct_api_a0 *api,
                                          int reload) {
    CT_UNUSED(api);

    ct_log_a0->info("example", "Init %d", reload);

    api->register_api(GAME_INTERFACE_NAME, &game_i0);

}

void CETECH_MODULE_UNLOAD (example_develop)(struct ct_api_a0 *api,
                                            int reload) {
    CT_UNUSED(api);

    ct_log_a0->info("example", "Shutdown %d", reload);
}

