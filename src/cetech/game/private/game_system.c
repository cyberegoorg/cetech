#include <celib/memory/allocator.h>
#include <celib/api.h>
#include <celib/module.h>
#include <celib/macros.h>
#include <celib/id.h>

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <stddef.h>

#include <cetech/kernel/kernel.h>
#include <celib/containers/hash.h>
#include <celib/memory/memory.h>
#include <celib/cdb.h>
#include <cetech/debugui/debugui.h>
#include <cetech/ecs/ecs.h>
#include <celib/config.h>
#include <cetech/transform/transform.h>
#include <cetech/camera/camera.h>

#include "../game_system.h"

#define _G game_system_global

typedef struct game_state_t {
    ct_world_t0 world;
    ct_viewport_t0 viewport;
    ct_entity_t0 camera_ent;
    ct_entity_t0 main_ent;
    bool started;
} game_state_t;

struct _G {
    ce_hash_t game_paused;
    ce_hash_t game_interface_map;
    ct_game_i0 **game_interface;
    game_state_t game_state;
    uint64_t active_game;
} _G;

static struct ct_game_i0 *_get_game(uint64_t name) {
    return (ct_game_i0 *) ce_hash_lookup(&_G.game_interface_map, name, 0);
}

#define _CAMERA_ASSET 0x57899875c4457313

static void game_init() {
    _G.game_state.world = ct_ecs_a0->create_world();

    uint64_t boot_ent = ce_config_a0->read_uint(CONFIG_BOOT_ENT, 0);

    _G.game_state.main_ent = ct_ecs_a0->spawn(_G.game_state.world, boot_ent);

    struct ct_game_i0 *gi = _get_game(_G.active_game);
    gi->init();
}

static void game_shutdown() {
    struct ct_game_i0 *gi = _get_game(_G.active_game);
    gi->shutdown();

    ct_ecs_a0->destroy_world(_G.game_state.world);
}

static void game_step(uint64_t name,
                      float dt) {
    ct_game_i0 *game_i = _get_game(name);

    if (!game_i) {
        return;
    }

    if (!game_i->update) {
        return;
    }

    game_i->update(dt);

    ct_ecs_a0->simulate(_G.game_state.world, dt);

    ct_transform_comp *t = ct_ecs_a0->get_one(_G.game_state.world, TRANSFORM_COMPONENT,
                                              _G.game_state.camera_ent);
    ct_camera_component *c = ct_ecs_a0->get_one(_G.game_state.world, CT_CAMERA_COMPONENT,
                                                _G.game_state.camera_ent);

    ct_renderer_a0->viewport_render(_G.game_state.viewport,
                                    _G.game_state.world,
                                    (ct_camera_data_t0) {
                                            .world = t->world,
                                            .camera = *c,
                                    });

}

static bool game_is_paused(uint64_t name) {
    return ce_hash_contain(&_G.game_paused, name);
}

static void game_update(float dt) {
    if (game_is_paused(_G.active_game)) {
        return;
    }

    struct ct_game_i0 *gi = _get_game(_G.active_game);
    gi->update(dt);

    ct_ecs_a0->simulate(_G.game_state.world, dt);

    ct_transform_comp *t = ct_ecs_a0->get_one(_G.game_state.world, TRANSFORM_COMPONENT,
                                              _G.game_state.camera_ent);
    ct_camera_component *c = ct_ecs_a0->get_one(_G.game_state.world, CT_CAMERA_COMPONENT,
                                                _G.game_state.camera_ent);

    ct_renderer_a0->viewport_render(_G.game_state.viewport,
                                    _G.game_state.world,
                                    (ct_camera_data_t0) {
                                            .world = t->world,
                                            .camera = *c,
                                    });
}

static struct ct_viewport_t0 game_render_graph_builder(uint64_t name) {
    return _G.game_state.viewport;
}

static void game_pause(uint64_t name) {
    ce_hash_add(&_G.game_paused, name, true, ce_memory_a0->system);
}

static void game_play(uint64_t name) {
    ce_hash_remove(&_G.game_paused, name);
}

static void _game_api_add(uint64_t name,
                          void *api) {
    ct_game_i0 *game_i = api;

    ce_hash_add(&_G.game_interface_map, game_i->name(),
                (uint64_t) game_i, ce_memory_a0->system);

    ce_array_push(_G.game_interface, game_i, ce_memory_a0->system);

}

struct ct_game_system_a0 game_system_api = {
        .pause = game_pause,
        .is_paused = game_is_paused,
        .play = game_play,
        .step = game_step,
        .viewport = game_render_graph_builder,
};

struct ct_game_system_a0 *ct_game_system_a0 = &game_system_api;


static uint64_t task_name() {
    return CT_GAME_TASK;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_INPUT_TASK,
            CT_DEBUGUI_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}


static struct ct_kernel_task_i0 game_task = {
        .name = task_name,
        .update = game_update,
        .update_after = update_after,
        .init = game_init,
        .shutdown = game_shutdown,
};

//==============================================================================
// Module def
//==============================================================================

void CE_MODULE_LOAD (game_system)(struct ce_api_a0 *api,
                                  int reload) {
    _G = (struct _G) {
            .active_game = ce_id_a0->id64("default"),
    };

    api->register_api(CT_GAME_SYSTEM_API, ct_game_system_a0, sizeof(game_system_api));
    api->register_api(KERNEL_TASK_INTERFACE, &game_task, sizeof(game_task));

    ce_api_a0->register_on_add(GAME_INTERFACE, _game_api_add);

    CE_UNUSED(api);
}

void CE_MODULE_UNLOAD (game_system)(struct ce_api_a0 *api,
                                    int reload) {
    CE_UNUSED(api);
}


