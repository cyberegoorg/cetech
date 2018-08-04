#include <string.h>

#include <corelib/ebus.h>
#include <corelib/macros.h>
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/module.h"

#include "corelib/api_system.h"

#include <cetech/gfx/debugui.h>
#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/editor/selected_object.h>
#include <cetech/kernel/kernel.h>
#include <cetech/gfx/renderer.h>
#include <cetech/editor/dock.h>
#include <cetech/game_system/game_system.h>

#define WINDOW_NAME "Game view"

static float step_dt = 16.0f;
static void on_debugui(struct ct_dock_i0 *dock) {
    float size[2];
    ct_debugui_a0->GetContentRegionAvail(size);

    uint64_t game_name = ct_hashlib_a0->id64("default");

    struct ct_render_graph_builder* rgb = ct_game_system_a0->render_graph_builder(game_name);
    rgb->call->set_size(rgb, size[0], size[1]);

    const char* label[] = {
            ICON_FA_PAUSE,
            ICON_FA_PLAY,
    };

    bool is_paused = ct_game_system_a0->is_paused(game_name);

    if(ct_debugui_a0->Button(label[is_paused], (float[2]) {0.0f})) {
        if(is_paused) {
            ct_game_system_a0->play(game_name);
        } else {
            ct_game_system_a0->pause(game_name);
        }
    }

    if(is_paused) {
        ct_debugui_a0->SameLine(0.0f, 4.0f);
        if(ct_debugui_a0->Button(ICON_FA_FORWARD, (float[2]) {0.0f})) {
            ct_game_system_a0->step(game_name, step_dt);
        }

        ct_debugui_a0->SameLine(0.0f, 4.0f);
        ct_debugui_a0->SliderFloat("delta", &step_dt, 0.0f, 33.0f, NULL, 1.0f);
    }


    ct_render_texture_handle_t th;
    th = rgb->call->get_texture(rgb, RG_OUTPUT_TEXTURE);

    ct_debugui_a0->Image(th,
                         size,
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});

}

static const char *dock_title() {
    return ICON_FA_GAMEPAD " " WINDOW_NAME;
}

static const char *name(struct ct_dock_i0 *dock) {
    return "game_view";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .id = 0,
        .dock_flag = DebugUIWindowFlags_NoNavInputs |
                     DebugUIWindowFlags_NoScrollbar |
                     DebugUIWindowFlags_NoScrollWithMouse,
        .visible = true,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
};

static void _init(struct ct_api_a0 *api) {
    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);

}

static void _shutdown() {
}

CETECH_MODULE_DEF(
        game_view,
        {
            CT_INIT_API(api, ct_hashlib_a0);
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