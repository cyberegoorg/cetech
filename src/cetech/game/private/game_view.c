#include <string.h>


#include <celib/macros.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/module.h"

#include "celib/api.h"


#include <cetech/ui/icons_font_awesome.h>
#include <cetech/renderer/renderer.h>
#include <cetech/editor/dock.h>
#include <cetech/renderer/gfx.h>
#include <cetech/game/game_system.h>


#include <cetech/render_graph/render_graph.h>
#include <cetech/ecs/ecs.h>
#include <cetech/ui/ui.h>

#define WINDOW_NAME "Game view"

static float step_dt = 16.0f;

static void on_menu(uint64_t content,
                    uint64_t context,
                    uint64_t selected_object) {
    static const char *label[] = {
            ICON_FA_PAUSE,
            ICON_FA_PLAY,
    };

    bool is_paused = ct_game_system_a0->is_paused();


    if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=label[is_paused]})) {
        if (is_paused) {
            ct_game_system_a0->play();
        } else {
            ct_game_system_a0->pause();
        }
    }

    if (is_paused) {
        ct_ui_a0->same_line(0.0f, 4.0f);
        if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=ICON_FA_FORWARD})) {
            ct_game_system_a0->step(step_dt / 1000.0f);
        }

        ct_ui_a0->same_line(0.0f, 4.0f);

        ct_ui_a0->drag_float(&(struct ct_ui_drag_float_t0) {
                .id = ce_id_a0->id64("game_delta")
        }, &step_dt);

    }

}

static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    ce_vec2_t size = ct_ui_a0->get_content_region_avail();

    ct_world_t0 world = ct_game_system_a0->world();
    ct_entity_t0 viewport_ent = ct_ecs_q_a0->first(world, (ct_ecs_query_t0) {
            .all = CT_ECS_ARCHETYPE(VIEWPORT_COMPONENT),
    });

    viewport_component *viewport = ct_ecs_c_a0->get_one(world, VIEWPORT_COMPONENT,
                                                        viewport_ent, false);

    if (viewport) {
        ct_rg_builder_t0 *builder;
        builder = ct_renderer_a0->viewport_builder(viewport->viewport);
        ct_renderer_a0->viewport_set_size(viewport->viewport, size);

        bgfx_texture_handle_t th;
        th = builder->get_texture(builder, RG_OUTPUT_TEXTURE);

        ct_ui_a0->image(&(struct ct_ui_image_t0) {
                .user_texture_id = th.idx,
                .size = size,
                .tint_col = {1.0f, 1.0f, 1.0f, 1.0f},
                .border_col ={0.0f, 0.0f, 0.0f, 0.0f},
        });
    }
}

static const char *dock_title(uint64_t content,
                              uint64_t selected_object) {
    return ICON_FA_GAMEPAD " " WINDOW_NAME;
}

static const char *name() {
    return "game_view";
}

static uint64_t dock_flags() {
    return CT_UI_WINDOW_FLAGS_NoNavInputs |
           CT_UI_WINDOW_FLAGS_NoScrollbar |
           CT_UI_WINDOW_FLAGS_NoScrollWithMouse;
}

static struct ct_dock_i0 dock_api = {
        .type =  CT_GAME_I0,
        .ui_flags = dock_flags,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
        .draw_menu = on_menu,
};


void CE_MODULE_LOAD(game_view)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_id_a0);

    api->add_impl(CT_DOCK_I0_STR, &dock_api, sizeof(dock_api));

    ct_dock_a0->create_dock(CT_GAME_I0, true);
}

void CE_MODULE_UNLOAD(game_view)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
