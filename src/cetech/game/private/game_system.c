#include <celib/memory/allocator.h>
#include <celib/api.h>
#include <celib/module.h>
#include <celib/macros.h>

#include <cetech/renderer/renderer.h>

#include <cetech/kernel/kernel.h>
#include <cetech/debugui/debugui.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <cetech/camera/camera.h>

#include "../game_system.h"

#define _G game_system_global

typedef struct game_state_t {
    ct_world_t0 world;
    bool started;
} game_state_t;

struct _G {
    bool game_paused;
    game_state_t game_state;
    ct_game_i0* game;
} _G;

static void game_init() {
    _G.game_state.world = ct_ecs_a0->create_world();
    struct ct_game_i0 *gi =_G.game;
    if(!gi) {
        return;
    }

    gi->init(_G.game_state.world);
}

static void game_shutdown() {
    struct ct_game_i0 *gi = _G.game;
    if(!gi) {
        return;
    }

    gi->shutdown(_G.game_state.world);
    ct_ecs_a0->destroy_world(_G.game_state.world);
}

static void game_step(float dt) {
    ct_game_i0 *game_i = _G.game;

    if (!game_i) {
        return;
    }

    if (!game_i->update) {
        return;
    }

    game_i->update(_G.game_state.world, dt);
    ct_ecs_a0->step(_G.game_state.world, dt);
}

static bool game_is_paused() {
    return _G.game_paused;
}

static void game_update(float dt) {
    if (game_is_paused()) {
        return;
    }

    game_step(dt);
}

static void game_pause() {
    _G.game_paused = true;
}

static ct_world_t0 game_world() {
    return _G.game_state.world;
}

static void game_play() {
    _G.game_paused = false;
}

static void _game_api_add(uint64_t name,
                          void *api) {
    if (CT_GAME_I == name) {
        ct_game_i0 *game_i = api;

        if(_G.game) {
            game_shutdown();
            _G.game = game_i;
            game_init();
        }

        _G.game = game_i;
    }
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
    };

    api->add_api(CT_GAME_SYSTEM_API, ct_game_system_a0, sizeof(game_system_api));
    api->add_impl(CT_KERNEL_TASK_I, &game_task, sizeof(game_task));

    ce_api_a0->register_on_add(_game_api_add);

    CE_UNUSED(api);
}

void CE_MODULE_UNLOAD (game_system)(struct ce_api_a0 *api,
                                    int reload) {
    CE_UNUSED(api);
}


