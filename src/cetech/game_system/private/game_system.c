#include <corelib/api_system.h>
#include <corelib/module.h>
#include <corelib/macros.h>
#include <corelib/hashlib.h>

#include <cetech/gfx/renderer.h>
#include <stddef.h>
#include <corelib/ebus.h>
#include <cetech/kernel/kernel.h>
#include <corelib/hash.inl>
#include <corelib/memory.h>

#include "../game_system.h"

#define _G game_system_global

struct _G {
    struct ct_hash_t game_interface_map;
    struct ct_hash_t game_paused;
    struct ct_game_i0** game_interface;
} _G;

static struct ct_game_i0 *_get_game(uint64_t name) {
    return (struct ct_game_i0*)ct_hash_lookup(&_G.game_interface_map, name, 0);
}

static void game_init() {
    const uint64_t game_n = ct_array_size(_G.game_interface);
    for (int i = 0; i < game_n; ++i) {
        _G.game_interface[i]->init();
    }
}

static void game_shutdown() {
    const uint64_t game_n = ct_array_size(_G.game_interface);
    for (int i = 0; i < game_n; ++i) {
        _G.game_interface[i]->shutdown();
    }
}

static void game_step(uint64_t name, float dt) {
    struct ct_game_i0 *game_i = _get_game(name);

    if (!game_i) {
        return;
    }

    if (!game_i->update) {
        return;
    }


    game_i->update(dt);
}

static bool game_is_paused(uint64_t name) {
    return ct_hash_contain(&_G.game_paused, name);
}

static void game_update(uint64_t event) {
    float dt = ct_cdb_a0->read_float(event, KERNEL_EVENT_DT, 0.0f);

    const uint64_t game_n = ct_array_size(_G.game_interface);
    for (int i = 0; i < game_n; ++i) {
        struct ct_game_i0* gi = _G.game_interface[i];

        if (game_is_paused(gi->name())) {
            continue;
        }

        gi->update(dt);
    }
}

static struct ct_render_graph_builder *game_render_graph_builder(uint64_t name) {
    struct ct_game_i0 *game_i = _get_game(name);

    if (!game_i) {
        return NULL;
    }

    return game_i->render_graph_builder();
}


static void game_render() {
    const uint64_t game_n = ct_array_size(_G.game_interface);
    for (int i = game_n-1; i >= 0; --i) {
        struct ct_game_i0 *gi = _G.game_interface[i];
        if (!gi->render) {
            return;
        }

        if (!gi->render_graph_builder) {
            continue;
        }

        if (!gi->render_graph_builder()) {
            continue;
        }

        gi->render();
    }
}

static void game_pause(uint64_t name) {
    ct_hash_add(&_G.game_paused, name, true, ct_memory_a0->system);
}

static void game_play(uint64_t name) {
        ct_hash_remove(&_G.game_paused, name);
}

static void _game_api_add(uint64_t name,
                          void *api) {
    struct ct_game_i0 *game_i = api;

    ct_hash_add(&_G.game_interface_map, game_i->name(),
                (uint64_t) game_i, ct_memory_a0->system);

    ct_array_push(_G.game_interface, game_i, ct_memory_a0->system);

}

struct ct_game_system_a0 game_system_api = {
        .pause = game_pause,
        .is_paused = game_is_paused,
        .play = game_play,
        .step = game_step,
        .render_graph_builder = game_render_graph_builder,
};

struct ct_game_system_a0 *ct_game_system_a0 = &game_system_api;


//==============================================================================
// Module def
//==============================================================================
void CETECH_MODULE_INITAPI(game_system)(struct ct_api_a0 *api) {

}

void CETECH_MODULE_LOAD (game_system)(struct ct_api_a0 *api,
                                      int reload) {

    api->register_api("ct_game_system_a0", ct_game_system_a0);

    ct_ebus_a0->connect(RENDERER_EBUS, RENDERER_RENDER_EVENT,
                        game_render, GAME_ORDER);

    ct_ebus_a0->connect(KERNEL_EBUS, KERNEL_INIT_EVENT,
                        game_init, GAME_ORDER);

    ct_ebus_a0->connect(KERNEL_EBUS, KERNEL_SHUTDOWN_EVENT,
                        game_shutdown, GAME_ORDER);

    ct_ebus_a0->connect(KERNEL_EBUS, KERNEL_UPDATE_EVENT,
                        game_update, GAME_ORDER);

    ct_api_a0->register_on_add(GAME_INTERFACE, _game_api_add);

    CT_UNUSED(api);
}

void CETECH_MODULE_UNLOAD (game_system)(struct ct_api_a0 *api,
                                        int reload) {
    CT_UNUSED(api);
}


