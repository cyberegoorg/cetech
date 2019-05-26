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
//    ct_entity_t0 camera_ent;
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

static void game_init() {
    _G.game_state.world = ct_ecs_a0->create_world();

    uint64_t boot_ent = ce_config_a0->read_uint(CONFIG_BOOT_ENT, 0);

    _G.game_state.main_ent = ct_ecs_a0->spawn(_G.game_state.world, boot_ent);

    ce_cdb_a0->log_obj("ENT", ce_cdb_a0->db(), boot_ent);

    struct ct_game_i0 *gi = _get_game(_G.active_game);
    gi->init();
}

static void game_shutdown() {
    struct ct_game_i0 *gi = _get_game(_G.active_game);
    gi->shutdown();

    ct_ecs_a0->destroy_world(_G.game_state.world);
}

typedef struct main_camera_data_t {
    ct_entity_t0 ent;
    ct_viewport_t0 viewport;
    ct_camera_component camera;
    ce_mat4_t world;
} main_camera_data_t;

static void _get_active_camera(struct ct_world_t0 world,
                               struct ct_entity_t0 *ent,
                               ct_entity_storage_o0 *item,
                               uint32_t n,
                               ct_ecs_cmd_buffer_t *buff,
                               void *data) {
    main_camera_data_t *output = data;

    ct_entity_t0 camera_ent = ent[0];

    viewport_component *viewports = ct_ecs_a0->get_all(VIEWPORT_COMPONENT, item);
    ct_camera_component *cameras = ct_ecs_a0->get_all(CT_CAMERA_COMPONENT, item);
    ct_transform_comp *tramsforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);

    output->ent = camera_ent;
    output->viewport = viewports[0].viewport;
    output->camera = cameras[0];
    output->world = tramsforms[0].world;
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

    ct_ecs_a0->step(_G.game_state.world, dt);

    main_camera_data_t camera_data = {};
    ct_ecs_a0->process(_G.game_state.world,
                       ct_ecs_a0->mask(VIEWPORT_COMPONENT)
                       | ct_ecs_a0->mask(TRANSFORM_COMPONENT)
                       | ct_ecs_a0->mask(CT_CAMERA_COMPONENT)
                       | ct_ecs_a0->mask(CT_ACTIVE_CAMERA_COMPONENT),
                       _get_active_camera, &camera_data);

    ct_renderer_a0->viewport_render(camera_data.viewport,
                                    _G.game_state.world,
                                    (ct_camera_data_t0) {
                                            .world = camera_data.world,
                                            .camera = camera_data.camera,
                                    });

}

static bool game_is_paused(uint64_t name) {
    return ce_hash_contain(&_G.game_paused, name);
}

static void game_update(float dt) {
    if (game_is_paused(_G.active_game)) {
        return;
    }

    game_step(_G.active_game, dt);
}

static void game_pause(uint64_t name) {
    ce_hash_add(&_G.game_paused, name, true, ce_memory_a0->system);
}

static ct_world_t0 game_world() {
    return _G.game_state.world;
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
        .world = game_world,
        .pause = game_pause,
        .is_paused = game_is_paused,
        .play = game_play,
        .step = game_step,
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
    api->add_impl(CT_KERNEL_TASK_I, &game_task, sizeof(game_task));

    ce_api_a0->register_on_add(CT_GAME_I, _game_api_add);

    CE_UNUSED(api);
}

void CE_MODULE_UNLOAD (game_system)(struct ce_api_a0 *api,
                                    int reload) {
    CE_UNUSED(api);
}


