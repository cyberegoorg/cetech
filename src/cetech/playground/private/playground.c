#include <stdio.h>

#include <corelib/macros.h>
#include <corelib/allocator.h>
#include <corelib/fs.h>
#include <corelib/os.h>
#include <corelib/ydb.h>
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"

#include <corelib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/playground/playground.h>
#include <cetech/camera/camera.h>
#include <cetech/playground/command_system.h>
#include <cetech/playground/action_manager.h>
#include <corelib/ebus.h>
#include <cetech/kernel/kernel.h>
#include <cetech/render_graph/render_graph.h>

#define _G plaground_global

static struct _G {
    bool load_layout;
    struct ct_render_graph *render_graph;
    struct ct_render_graph_builder *render_graph_builder;
    struct ct_render_graph_module *module;
} _G;

void reload_layout() {
    _G.load_layout = true;
}

static float draw_main_menu() {
    float menu_height = 0;
    static bool debug = false;

    if (ct_debugui_a0->BeginMainMenuBar()) {
        if (ct_debugui_a0->BeginMenu("File", true)) {
            if (ct_debugui_a0->MenuItem("Reload", "Alt+r", false, true)) {
                ct_module_a0->reload_all();
            }

            if (ct_debugui_a0->MenuItem("Save", "Alt+s", false, true)) {
                ct_ydb_a0->save_all_modified();
            }

            if (ct_debugui_a0->MenuItem2("Debug", "F9", &debug, true)) {
                ct_renderer_a0->set_debug(debug);
            }

            if (ct_debugui_a0->MenuItem("Quit", "Alt+F4", false, true)) {
                uint64_t event = ct_cdb_a0->create_object(
                        ct_cdb_a0->global_db(),
                        KERNEL_QUIT_EVENT);
                ct_ebus_a0->broadcast(KERNEL_EBUS, event);

            }

            ct_debugui_a0->EndMenu();
        }

        if (ct_debugui_a0->BeginMenu("Edit", true)) {
            char buffer[128];
            char buffer2[128];

            ct_cmd_system_a0->undo_text(buffer2, CT_ARRAY_LEN(buffer2));
            const char *shortcut;

            sprintf(buffer, "Undo %s", buffer2[0] != '0' ? buffer2 : "");

            shortcut = ct_action_manager_a0->shortcut_str(CT_ID64_0("undo"));
            if (ct_debugui_a0->MenuItem(buffer, shortcut, false,
                                        buffer2[0] != '0')) {
                ct_action_manager_a0->execute(CT_ID64_0("undo"));
            }


            ct_cmd_system_a0->redo_text(buffer2, CT_ARRAY_LEN(buffer2));
            shortcut = ct_action_manager_a0->shortcut_str(CT_ID64_0("redo"));
            sprintf(buffer, "Redo %s", buffer2[0] != '0' ? buffer2 : "");
            if (ct_debugui_a0->MenuItem(buffer, shortcut, false,
                                        buffer2[0] != '0')) {
                ct_action_manager_a0->execute(CT_ID64_0("redo"));
            }

            ct_debugui_a0->EndMenu();
        }

        if (ct_debugui_a0->BeginMenu("Window", true)) {
            if (ct_debugui_a0->BeginMenu("Layout", true)) {
                if (ct_debugui_a0->MenuItem("Save", NULL, false, true)) {
                    struct ct_vio *f = ct_fs_a0->open(CT_ID64_0("source"),
                                                      "core/default.dock_layout",
                                                      FS_OPEN_WRITE);
                    ct_debugui_a0->SaveDock(f);
                    ct_fs_a0->close(f);
                }

                if (ct_debugui_a0->MenuItem("Load", NULL, false, true)) {
                    ct_debugui_a0->LoadDock("core/default.dock_layout");
                }
                ct_debugui_a0->EndMenu();
            }

            ct_debugui_a0->Separator();

            struct ct_api_entry it = ct_api_a0->first("ct_dock_i");
            while (it.api) {
                struct ct_dock_i *i = (it.api);

                char title[128] = {0};

                snprintf(title, CT_ARRAY_LEN(title), "%s %llu", i->title(i), i->id);

                ct_debugui_a0->MenuItem2(title, NULL, &i->visible, true);

                it = ct_api_a0->next(it);
            }

            ct_debugui_a0->EndMenu();
        }


        struct ct_api_entry it = ct_api_a0->first("ct_dock_i");
        while (it.api) {
            struct ct_dock_i *i = (it.api);

            if(i->draw_main_menu) {
                i->draw_main_menu();
            }
            it = ct_api_a0->next(it);
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
    struct ct_api_entry it = ct_api_a0->first("ct_dock_i");
    while (it.api) {
        struct ct_dock_i *i = (it.api);


        char title[128] = {0};
        snprintf(title, CT_ARRAY_LEN(title), "%s##dock%llu", i->title(i), i->id);

        if (ct_debugui_a0->BeginDock(title, &i->visible, 0)) {
            if(i->draw_ui) {
                i->draw_ui(i);
            }
        }
        ct_debugui_a0->EndDock();

        it = ct_api_a0->next(it);
    }
}

static void debugui_on_setup(void *inst,
                             struct ct_render_graph_builder *builder) {
    builder->call->add_pass(builder, inst, 0);
}

static void debugui_on_pass(void *inst,
                            uint8_t viewid,
                            uint64_t layer,
                            struct ct_render_graph_builder *builder) {
    ct_debugui_a0->render(viewid);
}


static void on_init(uint64_t _event) {
    uint64_t event = ct_cdb_a0->create_object(ct_cdb_a0->global_db(),
                                              PLAYGROUND_INIT_EVENT);

    ct_ebus_a0->broadcast(PLAYGROUND_EBUS, event);


    _G.render_graph = ct_render_graph_a0->create_graph();
    _G.render_graph_builder = ct_render_graph_a0->create_builder();
    _G.module = ct_render_graph_a0->create_module();

    static struct ct_render_graph_pass debugui_pass = {
            .on_pass = debugui_on_pass,
            .on_setup = debugui_on_setup
    };

    _G.module->call->add_pass(_G.module, &debugui_pass,
                              sizeof(struct ct_render_graph_pass));

    _G.render_graph->call->add_module(_G.render_graph, _G.module);
}

static void on_shutdown(uint64_t _event) {
    uint64_t event = ct_cdb_a0->create_object(ct_cdb_a0->global_db(),
                                              PLAYGROUND_SHUTDOWN_EVENT);

    ct_ebus_a0->broadcast(PLAYGROUND_EBUS, event);
}

static void on_update(uint64_t app_event) {
    ct_action_manager_a0->check();


    uint64_t event = ct_cdb_a0->create_object(ct_cdb_a0->global_db(),
                                              PLAYGROUND_UPDATE_EVENT);


    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(event);
    ct_cdb_a0->set_float(w, CT_ID64_0("dt"),
                         ct_cdb_a0->read_float(app_event, CT_ID64_0("dt"),
                                               0.0f));
    ct_cdb_a0->write_commit(w);

    ct_ebus_a0->broadcast(PLAYGROUND_EBUS, event);
}

static void on_render(uint64_t _event) {
    uint64_t event = ct_cdb_a0->create_object(ct_cdb_a0->global_db(),
                                              PLAYGROUND_RENDER_EVENT);

    ct_ebus_a0->broadcast(PLAYGROUND_EBUS, event);

    _G.render_graph_builder->call->clear(_G.render_graph_builder);
    _G.render_graph->call->setup(_G.render_graph, _G.render_graph_builder);
    _G.render_graph_builder->call->execute(_G.render_graph_builder);
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


static struct ct_playground_a0 playground_api = {
        .reload_layout = reload_layout,
};

struct ct_playground_a0 *ct_playground_a0 = &playground_api;

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .load_layout = true,
    };

    api->register_api("ct_playground_a0", &playground_api);

    ct_ebus_a0->create_ebus(PLAYGROUND_EBUS_NAME, PLAYGROUND_EBUS);

    ct_action_manager_a0->register_action(
            CT_ID64_0("undo"),
            "ctrl+z",
            ct_cmd_system_a0->undo
    );

    ct_action_manager_a0->register_action(
            CT_ID64_0("redo"),
            "ctrl+shift+z",
            ct_cmd_system_a0->redo
    );


    ct_ebus_a0->connect(KERNEL_EBUS, KERNEL_INIT_EVENT, on_init, GAME_ORDER);
    ct_ebus_a0->connect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, on_update,
                        KERNEL_ORDER);
    ct_ebus_a0->connect(KERNEL_EBUS, KERNEL_SHUTDOWN_EVENT, on_shutdown,
                        KERNEL_ORDER);
    ct_ebus_a0->connect(RENDERER_EBUS, RENDERER_RENDER_EVENT, on_render, 0);
    ct_ebus_a0->connect(DEBUGUI_EBUS, DEBUGUI_EVENT, on_ui, 1);
}

static void _shutdown() {
    ct_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_INIT_EVENT, on_init);
    ct_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, on_update);
    ct_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_SHUTDOWN_EVENT, on_shutdown);
    ct_ebus_a0->disconnect(KERNEL_EBUS, RENDERER_RENDER_EVENT, on_render);
    ct_ebus_a0->disconnect(DEBUGUI_EBUS, DEBUGUI_EVENT, on_ui);

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        playground,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_action_manager_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
            CETECH_GET_API(api, ct_module_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_render_graph_a0);
            CETECH_GET_API(api, ct_cdb_a0);
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