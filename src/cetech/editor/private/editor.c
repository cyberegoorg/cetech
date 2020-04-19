

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/fs.h>

#include <celib/cdb_yaml.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"

#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>

#include <cetech/editor/editor.h>
#include <cetech/camera/camera.h>
#include <cetech/editor/action_manager.h>

#include <cetech/kernel/kernel.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/editor/dock.h>
#include <string.h>
#include <cetech/game/game_system.h>
#include <cetech/ui/icons_font_awesome.h>
#include <cetech/metrics/metrics.h>
#include <cetech/ui/ui.h>

#define _G plaground_global

static struct _G {
    bool load_layout;
} _G;

static float draw_main_menu() {
    float menu_height = 0;
    static bool debug = false;;
    if (ct_ui_a0->begin_main_menu_bar()) {
        if (ct_ui_a0->menu_begin(&(ct_ui_menu_t0) {.text="File"})) {
            if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {.text="Reload", .shortcut= "Alt+r}"})) {
                ce_module_a0->reload_all();
            }


            if (ct_ui_a0->menu_item_checkbox(&(ct_ui_menu_item_t0) {
                    .text="Debug",
                    .shortcut= "F9"
            }, &debug)) {
                ct_renderer_a0->set_debug(debug);
            }


            if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {
                    .text=ICON_FA_WINDOW_CLOSE" ""Quit",
                    .shortcut= "Alt+F4}"})) {
                ct_kernel_a0->quit();

            }

            ct_ui_a0->menu_end();
        }

        if (ct_ui_a0->menu_begin(&(ct_ui_menu_t0) {.text="Edit"})) {
            ct_ui_a0->menu_end();
        }

        ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I0);
        while (it.api) {
            struct ct_editor_module_i0 *i = (it.api);

            if (i->draw_menu) {
                i->draw_menu();
            }

            it = ce_api_a0->next(it);
        }

        ct_dock_a0->draw_menu();

        if (ct_ui_a0->menu_begin(&(ct_ui_menu_t0) {.text="Help"})) {
            if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {.text="About"})) {
            }
            ct_ui_a0->menu_end();
        }

        ce_vec2_t v = ct_ui_a0->get_window_size();
        menu_height = v.y;

        float w = v.x;
        float dt = ct_metrics_a0->get_float(ce_id_a0->id64("dt"));
        ct_ui_a0->same_line(w - 120, 0);

        char text[32];
        snprintf(text, 32, "FPS: %.0f | DT: %.0f ms", 1.0f / dt, dt * 1000);
        ct_ui_a0->text(text);

        ct_ui_a0->end_main_menu_bar();
    }
    return menu_height;
}

static void on_init() {
    ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I0);
    while (it.api) {
        struct ct_editor_module_i0 *i = (it.api);
        if (i->init) {
            i->init();
        }
        it = ce_api_a0->next(it);
    }
}

static void on_shutdown() {
    ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I0);
    while (it.api) {
        struct ct_editor_module_i0 *i = (it.api);

        if (i->shutdown) {
            i->shutdown();
        }

        it = ce_api_a0->next(it);
    }
}

static void editor_task(float dt) {
    float menu_height = draw_main_menu();

    uint32_t w, h;
    ct_renderer_a0->get_size(&w, &h);
    ce_vec2_t pos = {0.0f, menu_height};
    ce_vec2_t size = {(float) w, h - 25.0f};

    ct_ui_a0->dock_root(pos, size);

    ct_dock_a0->draw_all();

    if (_G.load_layout) {
        ct_ui_a0->dock_load("core/default.dock_layout");
        _G.load_layout = false;
    }

    ct_action_manager_a0->check();

    ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I0);
    while (it.api) {
        struct ct_editor_module_i0 *i = (it.api);

        if (i->update) {
            i->update(dt);
        }

        it = ce_api_a0->next(it);
    }

}

struct ct_kernel_task_i0 render_task = {
        .name = CT_EDITOR_TASK,
        .update = editor_task,
        .init = on_init,
        .shutdown = on_shutdown,
        .update_after = CT_KERNEL_AFTER(CT_UI_TASK, CT_INPUT_TASK),
        .update_before = CT_KERNEL_BEFORE(CT_GAME_TASK, CT_RENDER_TASK),
};


void CE_MODULE_LOAD(playground)(struct ce_api_a0 *api,
                                int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_renderer_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ct_camera_a0);
    CE_INIT_API(api, ce_fs_a0);
    CE_INIT_API(api, ce_cdb_yaml_a0);
    CE_INIT_API(api, ct_action_manager_a0);
    CE_INIT_API(api, ce_module_a0);
    CE_INIT_API(api, ct_rg_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .load_layout = true,
    };

//    ce_api_a0->register_api(CT_GAME_I0, &editor_game_i0, sizeof(editor_game_i0));
    ce_api_a0->add_impl(CT_KERNEL_TASK_I0_STR, &render_task, sizeof(render_task));
}

void CE_MODULE_UNLOAD(playground)(struct ce_api_a0 *api,
                                  int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);


    _G = (struct _G) {};
}
