

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/fs.h>

#include <celib/ydb.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"

#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/editor.h>
#include <cetech/camera/camera.h>
#include <cetech/editor/action_manager.h>

#include <cetech/kernel/kernel.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/editor/dock.h>
#include <string.h>
#include <cetech/game/game_system.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/metrics/metrics.h>

#define _G plaground_global

static struct _G {
    bool load_layout;
} _G;

static float draw_main_menu() {
    float menu_height = 0;
    static bool debug = false;

    if (ct_debugui_a0->BeginMainMenuBar()) {
        if (ct_debugui_a0->BeginMenu("File", true)) {
            if (ct_debugui_a0->MenuItem("Reload", "Alt+r", false, true)) {
                ce_module_a0->reload_all();
            }

            if (ct_debugui_a0->MenuItem2("Debug", "F9", &debug, true)) {
                ct_renderer_a0->set_debug(debug);
            }

            if (ct_debugui_a0->MenuItem(ICON_FA_WINDOW_CLOSE" ""Quit", "Alt+F4", false, true)) {
                ct_kernel_a0->quit();

            }

            ct_debugui_a0->EndMenu();
        }

        if (ct_debugui_a0->BeginMenu("Edit", true)) {
            ct_debugui_a0->EndMenu();
        }

        ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I);
        while (it.api) {
            struct ct_editor_module_i0 *i = (it.api);

            if (i->draw_menu) {
                i->draw_menu();
            }

            it = ce_api_a0->next(it);
        }

        ct_dock_a0->draw_menu();

        if (ct_debugui_a0->BeginMenu("Help", true)) {
            if (ct_debugui_a0->MenuItem("About", NULL, false, true)) {
            }
            ct_debugui_a0->EndMenu();
        }

        float v[2];
        ct_debugui_a0->GetWindowSize(v);
        menu_height = v[1];

        float w = v[0];
        float dt = ct_metrics_a0->get_float(ce_id_a0->id64("dt"));
        ct_debugui_a0->SameLine(w - 120, 0);
        ct_debugui_a0->Text("FPS: %.0f | DT: %.0f ms", 1.0f / dt, dt * 1000);

        ct_debugui_a0->EndMainMenuBar();
    }
    return menu_height;
}

static void on_init() {
    ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I);
    while (it.api) {
        struct ct_editor_module_i0 *i = (it.api);
        if (i->init) {
            i->init();
        }
        it = ce_api_a0->next(it);
    }
}

static void on_shutdown() {
    ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I);
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

    ct_debugui_a0->RootDock(pos, size);

    ct_dock_a0->draw_all();

    if (_G.load_layout) {
        ct_debugui_a0->LoadDock("core/default.dock_layout");
        _G.load_layout = false;
    }

    ct_action_manager_a0->check();

    ce_api_entry_t0 it = ce_api_a0->first(CT_EDITOR_MODULE_I);
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
        .update_after = CT_KERNEL_AFTER(CT_DEBUGUI_TASK, CT_INPUT_TASK),
        .update_before = CT_KERNEL_BEFORE(CT_GAME_TASK, CT_RENDER_TASK),
};


void CE_MODULE_LOAD(playground)(struct ce_api_a0 *api,
                                int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_renderer_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ct_camera_a0);
    CE_INIT_API(api, ce_fs_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_action_manager_a0);
    CE_INIT_API(api, ce_module_a0);
    CE_INIT_API(api, ct_rg_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .load_layout = true,
    };

//    ce_api_a0->register_api(CT_GAME_I, &editor_game_i0, sizeof(editor_game_i0));
    ce_api_a0->add_impl(CT_KERNEL_TASK_I, &render_task, sizeof(render_task));
}

void CE_MODULE_UNLOAD(playground)(struct ce_api_a0 *api,
                                  int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);


    _G = (struct _G) {};
}
