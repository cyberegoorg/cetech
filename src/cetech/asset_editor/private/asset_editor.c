#include <stdio.h>
#include <string.h>

#include <corelib/cdb.h>
#include <corelib/ydb.h>
#include <corelib/fmath.inl>
#include <corelib/ebus.h>
#include <corelib/macros.h>
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"


#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/explorer/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_render_graph/default_render_graph.h>
#include <cetech/selected_object/selected_object.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/debugui/debugui.h>
#include <cetech/dock/dock.h>
#include <cetech/controlers/controlers.h>
#include <corelib/array.inl>
#include <cetech/asset_editor/asset_editor.h>

#define _G editor_globals


struct editor {
    char title[128];
    uint64_t type;
    uint64_t context_obj;
    struct ct_dock_i0 dock;
};

static struct _G {
    struct editor *editors;
} _G;


static struct ct_asset_editor_i0 *get_asset_editor(uint64_t cdb_type) {
    struct ct_api_entry it = ct_api_a0->first(ASSET_EDITOR_I);
    while (it.api) {
        struct ct_asset_editor_i0 *i = (it.api);

        if (cdb_type == i->asset_type()) {
            return i;
        }

        it = ct_api_a0->next(it);
    }

    return NULL;
};

static void draw_editor(struct ct_dock_i0 *dock) {
    struct editor *editor = &_G.editors[dock->id];

    struct ct_asset_editor_i0 *i = get_asset_editor(editor->type);

    if (!i) {
        return;
    }

    bool is_mouse_hovering = ct_debugui_a0->IsMouseHoveringWindow();
    bool click = ct_debugui_a0->IsMouseClicked(0, false);

    if (is_mouse_hovering && click) {
        uint64_t name = ct_cdb_a0->read_uint64(editor->context_obj,
                                               _ASSET_NAME, 0);
        uint64_t type = ct_cdb_a0->read_uint64(editor->context_obj,
                                               _ASSET_TYPE, 0);

        const char* path = ct_cdb_a0->read_str(editor->context_obj, _ASSET_PATH, 0);

        uint64_t selected_asset = ct_cdb_a0->create_object(ct_cdb_a0->db(),
                                                           ASSET_BROWSER_ASSET_TYPE);

        ct_cdb_obj_o *w = ct_cdb_a0->write_begin(selected_asset);
        ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_NAME, name);
        ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_TYPE2, type);
        ct_cdb_a0->set_str(w, ASSET_BROWSER_PATH, path);
        ct_cdb_a0->write_commit(w);


        ct_selected_object_a0->set_selected_object(selected_asset);
    }

    i->draw_ui(editor->context_obj);


}

static uint32_t find_editor(struct ct_resource_id asset) {
    const uint32_t editor_n = ct_array_size(_G.editors);

    for (uint32_t i = 0; i < editor_n; ++i) {
        struct editor *editor = &_G.editors[i];

        const uint64_t asset_name = ct_cdb_a0->read_uint64(editor->context_obj,
                                                           _ASSET_NAME, 0);

        const uint64_t asset_type = ct_cdb_a0->read_uint64(editor->context_obj,
                                                           _ASSET_TYPE, 0);

        if (!asset_name && !asset_type) {
            return i;
        }

        if ((asset_name == asset.name) && (asset_type == asset.type)) {
            return i;
        }
    }

    return UINT32_MAX;
}

#define DEFAULT_EDITOR_NAME  "Editor"

static const char *dock_title(struct ct_dock_i0 *dock) {
    struct editor *editor = &_G.editors[dock->id];
    struct ct_asset_editor_i0 *i = get_asset_editor(editor->type);

    if (!i) {
        return DEFAULT_EDITOR_NAME;
    }


    const char *display_icon = i->display_icon ? i->display_icon() : "";
    const char *display_name = i->display_name ? i->display_name()
                                               : DEFAULT_EDITOR_NAME;

    snprintf(editor->title, CT_ARRAY_LEN(editor->title),
             "%s %s", display_icon, display_name);

    return editor->title;
}

static const char *name(struct ct_dock_i0 *dock) {
    return "editor";
}

static struct editor *_new_editor(struct ct_resource_id asset) {
    uint32_t ent_idx = find_editor(asset);

    if (ent_idx != UINT32_MAX) {
        struct editor *editor = &_G.editors[ent_idx];
        return editor;
    }

    int idx = ct_array_size(_G.editors);
    ct_array_push(_G.editors, (struct editor) {}, ct_memory_a0->system);

    struct editor *editor = &_G.editors[idx];

    editor->context_obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), 0);

    editor->dock = (struct ct_dock_i0) {
            .id = idx,
            .dock_flag = DebugUIWindowFlags_NoNavInputs |
                         DebugUIWindowFlags_NoScrollbar |
                         DebugUIWindowFlags_NoScrollWithMouse,
            .visible = true,
            .display_title = dock_title,
            .name = name,
            .draw_ui = draw_editor,
    };


    ct_api_a0->register_api(DOCK_INTERFACE_NAME, &editor->dock);

    struct ct_world *w = ct_cdb_a0->write_begin(editor->context_obj);
    ct_cdb_a0->set_uint64(w, _ASSET_NAME, asset.name);
    ct_cdb_a0->set_uint64(w, _ASSET_TYPE, asset.type);
    ct_cdb_a0->write_commit(w);

    return editor;
}

static void open(struct ct_resource_id asset,
                 uint64_t root,
                 const char *path) {
    struct ct_asset_editor_i0 *i = get_asset_editor(asset.type);

    if (!i) {
        return;
    }

    struct editor *e = _new_editor(asset);
    e->type = asset.type;

    struct ct_world *w = ct_cdb_a0->write_begin(e->context_obj);
    ct_cdb_a0->set_uint64(w, _ASSET_NAME, asset.name);
    ct_cdb_a0->set_uint64(w, _ASSET_TYPE, asset.type);
    ct_cdb_a0->set_str(w, _ASSET_PATH, path);
    ct_cdb_a0->write_commit(w);

    i->open(e->context_obj);

}

static void update(float dt) {
    const uint32_t editor_n = ct_array_size(_G.editors);
    for (uint8_t i = 0; i < editor_n; ++i) {
        struct editor *editor = &_G.editors[i];

        struct ct_asset_editor_i0 *editor_i = get_asset_editor(editor->type);

        if (!editor_i) {
            return;
        }

        editor_i->update(editor->context_obj, dt);
    }
}

static void on_render() {
    const uint32_t editor_n = ct_array_size(_G.editors);
    for (uint8_t i = 0; i < editor_n; ++i) {
        struct editor *editor = &_G.editors[i];

        if (!editor->dock.visible) {
            continue;
        }

        struct ct_asset_editor_i0 *editor_i = get_asset_editor(editor->type);

        if (!editor_i) {
            return;
        }

        editor_i->render(editor->context_obj);
    }
}


static void on_asset_double_click(uint64_t event) {
    uint64_t asset_type = ct_cdb_a0->read_uint64(event,
                                                 ASSET_BROWSER_ASSET_TYPE2, 0);
    uint64_t asset_name = ct_cdb_a0->read_uint64(event,
                                                 ASSET_BROWSER_ASSET_NAME, 0);

    uint64_t root = ct_cdb_a0->read_uint64(event, ASSET_BROWSER_ROOT, 0);
    const char *path = ct_cdb_a0->read_str(event, ASSET_BROWSER_PATH, 0);

    struct ct_resource_id rid = {.name = asset_name, .type = asset_type};

    if (ENTITY_RESOURCE_ID == rid.type) {
        open(rid, root, path);
        return;
    }
}


static struct ct_editor_module_i0 ct_editor_module_i0 = {
        .update = update,
        .render= on_render,
};

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
    };

    ct_ebus_a0->connect(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT,
                        on_asset_double_click, 0);

    ct_api_a0->register_api("ct_editor_module_i0",
                            &ct_editor_module_i0);

    _new_editor((struct ct_resource_id) {.i128={0}});
}

static void _shutdown() {
    ct_ebus_a0->disconnect(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT,
                           on_asset_double_click);

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        editor,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_debugui_a0);
            CT_INIT_API(api, ct_ecs_a0);
            CT_INIT_API(api, ct_camera_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_ebus_a0);
            CT_INIT_API(api, ct_render_graph_a0);
            CT_INIT_API(api, ct_default_rg_a0);
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
