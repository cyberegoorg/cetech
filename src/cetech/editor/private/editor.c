#include <stdio.h>

#include <celib/macros.h>
#include <celib/allocator.h>
#include <celib/fs.h>
#include <celib/os.h>
#include <celib/ydb.h>
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"

#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/gfx/renderer.h>
#include <cetech/gfx/debugui.h>
#include <cetech/editor/editor.h>
#include <cetech/camera/camera.h>
#include <cetech/editor/command_system.h>
#include <cetech/editor/action_manager.h>
#include <celib/ebus.h>
#include <cetech/kernel/kernel.h>
#include <cetech/gfx/render_graph.h>
#include <cetech/editor/dock.h>
#include <cetech/editor/asset_browser.h>
#include <string.h>
#include <cetech/game_system/game_system.h>

#define _G plaground_global

static struct _G {
    bool load_layout;
    struct ct_world world;
    struct ct_entity render_ent;
} _G;

void reload_layout() {
    _G.load_layout = true;
}

#define _UNDO \
    CE_ID64_0("undo", 0xd9c7f03561492eecULL)

#define _REDO \
    CE_ID64_0("redo", 0x2b64b25d7febf67eULL)

static float draw_main_menu() {
    float menu_height = 0;
    static bool debug = false;

    if (ct_debugui_a0->BeginMainMenuBar()) {
        if (ct_debugui_a0->BeginMenu("File", true)) {
            if (ct_debugui_a0->MenuItem("Reload", "Alt+r", false, true)) {
                ce_module_a0->reload_all();
            }

            if (ct_debugui_a0->MenuItem("Save", "Alt+s", false, true)) {
                ce_ydb_a0->save_all_modified();
            }

            if (ct_debugui_a0->MenuItem2("Debug", "F9", &debug, true)) {
                ct_renderer_a0->set_debug(debug);
            }

            if (ct_debugui_a0->MenuItem("Quit", "Alt+F4", false, true)) {
                uint64_t event = ce_cdb_a0->create_object(
                        ce_cdb_a0->db(),
                        KERNEL_QUIT_EVENT);
                ce_ebus_a0->broadcast(KERNEL_EBUS, event);

            }

            ct_debugui_a0->EndMenu();
        }

        if (ct_debugui_a0->BeginMenu("Edit", true)) {
            char buffer[128];
            char buffer2[128];

            ct_cmd_system_a0->undo_text(buffer2, CE_ARRAY_LEN(buffer2));
            const char *shortcut;

            sprintf(buffer, "Undo %s", buffer2[0] != '0' ? buffer2 : "");

            shortcut = ct_action_manager_a0->shortcut_str(_UNDO);
            if (ct_debugui_a0->MenuItem(buffer, shortcut, false,
                                        buffer2[0] != '0')) {
                ct_action_manager_a0->execute(_UNDO);
            }


            ct_cmd_system_a0->redo_text(buffer2, CE_ARRAY_LEN(buffer2));
            shortcut = ct_action_manager_a0->shortcut_str(_REDO);
            sprintf(buffer, "Redo %s", buffer2[0] != '0' ? buffer2 : "");
            if (ct_debugui_a0->MenuItem(buffer, shortcut, false,
                                        buffer2[0] != '0')) {
                ct_action_manager_a0->execute(_REDO);
            }

            ct_debugui_a0->EndMenu();
        }

        if (ct_debugui_a0->BeginMenu("Window", true)) {
            if (ct_debugui_a0->BeginMenu("Layout", true)) {
                if (ct_debugui_a0->MenuItem("Save", NULL, false, true)) {
                    struct ce_vio *f = ce_fs_a0->open(ASSET_BROWSER_SOURCE,
                                                      "core/default.dock_layout",
                                                      FS_OPEN_WRITE);
                    ct_debugui_a0->SaveDock(f);
                    ce_fs_a0->close(f);
                }

                if (ct_debugui_a0->MenuItem("Load", NULL, false, true)) {
                    ct_debugui_a0->LoadDock("core/default.dock_layout");
                }
                ct_debugui_a0->EndMenu();
            }

            ct_debugui_a0->Separator();

            struct ce_api_entry it = ce_api_a0->first(DOCK_INTERFACE);
            while (it.api) {
                struct ct_dock_i0 *i = (it.api);

                char title[128] = {};

                snprintf(title, CE_ARRAY_LEN(title), "%s %llu",
                         i->display_title(i), i->id);

                ct_debugui_a0->MenuItem2(title, NULL, &i->visible, true);

                it = ce_api_a0->next(it);
            }

            ct_debugui_a0->EndMenu();
        }


        struct ce_api_entry it = ce_api_a0->first(DOCK_INTERFACE);
        while (it.api) {
            struct ct_dock_i0 *i = (it.api);

            if (i->draw_main_menu) {
                i->draw_main_menu();
            }
            it = ce_api_a0->next(it);
        }

        if (ct_debugui_a0->BeginMenu("Help", true)) {
            if (ct_debugui_a0->MenuItem("About", NULL, false, true)) {
            }
            ct_debugui_a0->EndMenu();
        }

        float v[2];
        ct_debugui_a0->GetWindowSize(v);
        menu_height = v[1];

        ct_debugui_a0->EndMainMenuBar();
    }
    return menu_height;
}

static void draw_all_docks() {
    struct ce_api_entry it = ce_api_a0->first(DOCK_INTERFACE);
    while (it.api) {
        struct ct_dock_i0 *i = (it.api);


        char title[128] = {};
        snprintf(title, CE_ARRAY_LEN(title), "%s##%s_dock%llu",
                 i->display_title(i), i->name(i), i->id);

        if (ct_debugui_a0->BeginDock(title, &i->visible, i->dock_flag)) {
            if (i->draw_menu) {
                i->draw_menu(i);
            }

            if (i->draw_ui) {
                i->draw_ui(i);
            }
        }
        ct_debugui_a0->EndDock();

        it = ce_api_a0->next(it);
    }
}


static void on_init(uint64_t _event) {
    struct ce_api_entry it = ce_api_a0->first(EDITOR_MODULE_INTERFACE);
    while (it.api) {
        struct ct_editor_module_i0 *i = (it.api);
        if (i->init) {
            i->init();
        }
        it = ce_api_a0->next(it);
    }

    _G.world = ct_ecs_a0->entity->create_world();

    ct_ecs_a0->entity->create(_G.world, &_G.render_ent, 1);
    ct_ecs_a0->component->add(_G.world, _G.render_ent,
                              (uint64_t[]) {RENDER_GRAPH_COMPONENT}, 1, NULL);

    struct ct_render_graph_component *rg_comp;
    rg_comp = ct_ecs_a0->component->get_one(_G.world, RENDER_GRAPH_COMPONENT,
                                            _G.render_ent);

    rg_comp->builder = ct_render_graph_a0->create_builder();
    rg_comp->graph = ct_render_graph_a0->create_graph();


    struct ct_render_graph_module *module = ct_render_graph_a0->create_module();
    rg_comp->graph->call->add_module(rg_comp->graph, module);
}

static void on_shutdown(uint64_t _event) {
    struct ce_api_entry it = ce_api_a0->first(EDITOR_MODULE_INTERFACE);
    while (it.api) {
        struct ct_editor_module_i0 *i = (it.api);

        if (i->shutdown) {
            i->shutdown();
        }

        it = ce_api_a0->next(it);
    }
}

static void on_update(float dt) {
    ct_action_manager_a0->check();

    struct ce_api_entry it = ce_api_a0->first(EDITOR_MODULE_INTERFACE);
    while (it.api) {
        struct ct_editor_module_i0 *i = (it.api);

        if (i->update) {
            i->update(dt);
        }

        it = ce_api_a0->next(it);
    }

    ct_ecs_a0->system->simulate(_G.world, dt);
}


static void on_ui(uint64_t _event) {
    float menu_height = draw_main_menu();

    uint32_t w, h;
    ct_renderer_a0->get_size(&w, &h);
    float pos[] = {0.0f, menu_height};
    float size[] = {(float) w, h - 25.0f};

    ct_debugui_a0->RootDock(pos, size);
    draw_all_docks();

    if (_G.load_layout) {
        ct_debugui_a0->LoadDock("core/default.dock_layout");
        _G.load_layout = false;
    }
}

static uint64_t name() {
    return ce_id_a0->id64("editor");
}

static struct ct_render_graph_builder *render_graph_builder() {
    struct ct_render_graph_component *rg_comp;
    rg_comp = ct_ecs_a0->component->get_one(_G.world, RENDER_GRAPH_COMPONENT,
                                            _G.render_ent);

    return rg_comp->builder;
}

struct ct_game_i0 editor_game_i0 = {
        .init = on_init,
        .shutdown = on_shutdown,
        .update = on_update,
        .name = name,
        .render_graph_builder = render_graph_builder
};

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .load_layout = true,
    };

    ct_action_manager_a0->register_action(
            CE_ID64_0("undo", 0xd9c7f03561492eecULL),
            "ctrl+z",
            ct_cmd_system_a0->undo
    );

    ct_action_manager_a0->register_action(
            CE_ID64_0("redo", 0x2b64b25d7febf67eULL),
            "ctrl+shift+z",
            ct_cmd_system_a0->redo
    );

    ce_api_a0->register_api(GAME_INTERFACE_NAME, &editor_game_i0);

    ce_ebus_a0->connect(DEBUGUI_EBUS, DEBUGUI_EVENT, on_ui, 1);
}

static void _shutdown() {
    ce_ebus_a0->disconnect(DEBUGUI_EBUS, DEBUGUI_EVENT, on_ui);

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        playground,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_renderer_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ct_camera_a0);
            CE_INIT_API(api, ce_fs_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ct_action_manager_a0);
            CE_INIT_API(api, ct_cmd_system_a0);
            CE_INIT_API(api, ce_module_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_render_graph_a0);
            CE_INIT_API(api, ce_cdb_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)