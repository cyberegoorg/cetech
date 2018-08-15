#include <stdio.h>
#include <string.h>

#include <celib/cdb.h>
#include <celib/ydb.h>
#include <celib/fmath.inl>
#include <celib/ebus.h>
#include <celib/macros.h>
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"


#include <cetech/ecs/ecs.h>
#include <cetech/gfx/renderer.h>
#include <cetech/gfx/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>
#include <cetech/gfx/render_graph.h>
#include <cetech/gfx/default_render_graph.h>
#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/gfx/debugui.h>
#include <cetech/editor/dock.h>
#include <cetech/controlers/controlers.h>
#include <celib/array.inl>
#include <cetech/editor/asset_editor.h>

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
    struct ce_api_entry it = ce_api_a0->first(ASSET_EDITOR_I);
    while (it.api) {
        struct ct_asset_editor_i0 *i = (it.api);

        if (cdb_type == i->asset_type()) {
            return i;
        }

        it = ce_api_a0->next(it);
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
        uint64_t name = ce_cdb_a0->read_uint64(editor->context_obj,
                                               _ASSET_NAME, 0);
        uint64_t type = ce_cdb_a0->read_uint64(editor->context_obj,
                                               _ASSET_TYPE, 0);

        uint64_t selected_asset;
        selected_asset = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                  ASSET_EDITOR_ASSET_SELECTED);

        ce_cdb_obj_o *w;
        w = ce_cdb_a0->write_begin(selected_asset);
        ce_cdb_a0->set_uint64(w, ASSET_NAME, name);
        ce_cdb_a0->set_uint64(w, ASSET_TYPE, type);
        ce_cdb_a0->write_commit(w);

        ce_ebus_a0->broadcast(ASSET_EDITOR_EBUS, selected_asset);
    }

    i->draw_ui(editor->context_obj);


}

static uint32_t find_editor(struct ct_resource_id asset) {
    const uint32_t editor_n = ce_array_size(_G.editors);

    for (uint32_t i = 0; i < editor_n; ++i) {
        struct editor *editor = &_G.editors[i];

        const uint64_t asset_name = ce_cdb_a0->read_uint64(editor->context_obj,
                                                           _ASSET_NAME, 0);

        const uint64_t asset_type = ce_cdb_a0->read_uint64(editor->context_obj,
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

    snprintf(editor->title, CE_ARRAY_LEN(editor->title),
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

    int idx = ce_array_size(_G.editors);
    ce_array_push(_G.editors, (struct editor) {}, ce_memory_a0->system);

    struct editor *editor = &_G.editors[idx];

    editor->context_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

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


    ce_api_a0->register_api(DOCK_INTERFACE_NAME, &editor->dock);

    struct ct_world *w = ce_cdb_a0->write_begin(editor->context_obj);
    ce_cdb_a0->set_uint64(w, _ASSET_NAME, asset.name);
    ce_cdb_a0->set_uint64(w, _ASSET_TYPE, asset.type);
    ce_cdb_a0->write_commit(w);

    return editor;
}

static void open(struct ct_resource_id asset,
                 uint64_t root) {
    struct ct_asset_editor_i0 *i = get_asset_editor(asset.type);

    if (!i) {
        return;
    }

    struct editor *e = _new_editor(asset);
    e->type = asset.type;

    struct ct_world *w = ce_cdb_a0->write_begin(e->context_obj);
    ce_cdb_a0->set_uint64(w, _ASSET_NAME, asset.name);
    ce_cdb_a0->set_uint64(w, _ASSET_TYPE, asset.type);
    ce_cdb_a0->write_commit(w);

    i->open(e->context_obj);

}

static void update(float dt) {
    const uint32_t editor_n = ce_array_size(_G.editors);
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
    const uint32_t editor_n = ce_array_size(_G.editors);
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
    uint64_t asset_type = ce_cdb_a0->read_uint64(event,
                                                 ASSET_TYPE, 0);
    uint64_t asset_name = ce_cdb_a0->read_uint64(event,
                                                 ASSET_NAME, 0);

    uint64_t root = ce_cdb_a0->read_uint64(event, ASSET_BROWSER_ROOT, 0);

    struct ct_resource_id rid = {.name = asset_name, .type = asset_type};

    if (ENTITY_RESOURCE_ID == rid.type) {
        open(rid, root);
        return;
    }
}


static struct ct_editor_module_i0 ct_editor_module_i0 = {
        .update = update,
        .render= on_render,
};

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
    };

    ce_ebus_a0->connect(ASSET_BROWSER_EBUS,
                        ASSET_DCLICK_EVENT,
                        on_asset_double_click, 0);

    ce_api_a0->register_api("ct_editor_module_i0",
                            &ct_editor_module_i0);

    ce_ebus_a0->create_ebus(ASSET_EDITOR_EBUS);

    _new_editor((struct ct_resource_id) {.i128={}});
}

static void _shutdown() {
    ce_ebus_a0->disconnect(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT,
                           on_asset_double_click);

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        editor,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ct_camera_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_render_graph_a0);
            CE_INIT_API(api, ct_default_rg_a0);
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
