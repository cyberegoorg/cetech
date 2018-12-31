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
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/editor.h>
#include <cetech/camera/camera.h>
#include <cetech/editor/action_manager.h>
#include <celib/ebus.h>
#include <cetech/kernel/kernel.h>
#include <cetech/renderer/gfx.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/editor/dock.h>
#include <cetech/editor/resource_browser.h>
#include <string.h>
#include <cetech/game/game_system.h>
#include <cetech/default_rg/default_rg.h>

#define _G plaground_global

static struct _G {
    bool load_layout;
    struct ct_world world;
    struct ct_viewport0 viewport;
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

            if (ct_debugui_a0->MenuItem2("Debug", "F9", &debug, true)) {
                ct_renderer_a0->set_debug(debug);
            }

            if (ct_debugui_a0->MenuItem("Quit", "Alt+F4", false, true)) {
                ce_ebus_a0->broadcast(KERNEL_EBUS, KERNEL_QUIT_EVENT, NULL, 0);

            }

            ct_debugui_a0->EndMenu();
        }

        if (ct_debugui_a0->BeginMenu("Edit", true)) {
            ct_debugui_a0->EndMenu();
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

        ct_debugui_a0->EndMainMenuBar();
    }
    return menu_height;
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

    _G.world = ct_ecs_a0->create_world();

//
//    struct ct_rg_builder *builder = ct_rg_a0->create_builder();
//    struct ct_rg *graph = ct_rg_a0->create_graph();
//
//    uint64_t rgc = ce_cdb_a0->create_object(ce_cdb_a0->db(),
//                                            RENDER_GRAPH_COMPONENT);
//
//
//    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(rgc);
//    ce_cdb_a0->set_ptr(w, PROP_RENDER_GRAPH_BUILDER, builder);
//    ce_cdb_a0->set_ptr(w, PROP_RENDER_GRAPH_GRAPH, graph);
//    ce_cdb_a0->write_commit(w);
//
//    ct_ecs_a0->create(_G.world, &_G.render_ent, 1);
//    ct_ecs_a0->add(_G.world, _G.render_ent,
//                   (uint64_t[]) {RENDER_GRAPH_COMPONENT}, 1,
//                   (uint64_t[]) {rgc});
//
//    struct ct_rg_module *module = ct_default_rg_a0->create(_G.world);
//    graph->add_module(graph, module);
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

    ct_ecs_a0->simulate(_G.world, dt);
}


static void on_ui(uint64_t _type, void* _event) {
    float menu_height = draw_main_menu();

    uint32_t w, h;
    ct_renderer_a0->get_size(&w, &h);
    float pos[] = {0.0f, menu_height};
    float size[] = {(float) w, h - 25.0f};

    ct_debugui_a0->RootDock(pos, size);

    ct_dock_a0->draw_all();

    if (_G.load_layout) {
        ct_debugui_a0->LoadDock("core/default.dock_layout");
        _G.load_layout = false;
    }
}

static uint64_t name() {
    return ce_id_a0->id64("editor");
}

//static struct ct_viewport0 render_graph_builder() {
//    return (struct ct_viewport0){};
//}

struct ct_game_i0 editor_game_i0 = {
        .init = on_init,
        .shutdown = on_shutdown,
        .update = on_update,
        .name = name,
//        .render_graph_builder = render_graph_builder
};

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .load_layout = true,
    };


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
            CE_INIT_API(api, ce_module_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_rg_a0);
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