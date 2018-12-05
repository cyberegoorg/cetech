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
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/asset_editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/render_graph/default_render_graph.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/dock.h>
#include <cetech/controlers/controlers.h>
#include <celib/array.inl>
#include <cetech/asset_editor/asset_editor.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/editor/dock.h>

#define _G editor_globals

#define _PROP_EDITOR\
    CE_ID64_0("editor", 0xf76c66a1ef2e2f59ULL)

#define _ASSET_EDITOR\
    CE_ID64_0("asset_editor", 0xd7ecca607e454ce9ULL)


struct editor {
    char title[128];
    uint64_t type;
    uint64_t obj;
    uint64_t context_obj;
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

static void draw_editor(uint64_t dock) {
    struct editor *editor = ce_cdb_a0->read_ptr(dock, _PROP_EDITOR, NULL);

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

        uint64_t obj = ct_resource_a0->get((struct ct_resource_id) {
                .type=type,
                .name=name});

        ct_selected_object_a0->set_selected_object(obj);
    }

    i->draw_ui(editor->context_obj);
}

static uint32_t find_editor(uint64_t obj) {
    const uint32_t editor_n = ce_array_size(_G.editors);

    for (uint32_t i = 0; i < editor_n; ++i) {
        struct editor *editor = &_G.editors[i];

        if (editor->obj == obj) {
            return i;
        }
    }

    return UINT32_MAX;
}

#define DEFAULT_EDITOR_NAME  "Editor"

static const char *dock_title(uint64_t dock) {
    struct editor *editor = ce_cdb_a0->read_ptr(dock, _PROP_EDITOR, NULL);

    if (!editor) {
        return NULL;
    }

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

static const char *name(uint64_t dock) {
    return "editor";
}

static struct editor *_get_or_create_editor(uint64_t obj) {
    uint32_t editor_idx = find_editor(obj);

    if (editor_idx != UINT32_MAX) {
        struct editor *editor = &_G.editors[editor_idx];
        return editor;
    }

    int idx = ce_array_size(_G.editors);
    ce_array_push(_G.editors, (struct editor) {}, ce_memory_a0->system);

    struct editor *editor = &_G.editors[idx];

    editor->context_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

    uint64_t dock = ct_dock_a0->create_dock(_ASSET_EDITOR,
                                            DebugUIWindowFlags_NoNavInputs |
                                            DebugUIWindowFlags_NoScrollbar |
                                            DebugUIWindowFlags_NoScrollWithMouse,
                                            true);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(dock);
    ce_cdb_a0->set_ptr(w, _PROP_EDITOR, editor);
    ce_cdb_a0->write_commit(w);

    return editor;
}

static void open(uint64_t obj) {
    uint64_t type = ce_cdb_a0->obj_type(obj);
    struct ct_asset_editor_i0 *i = get_asset_editor(type);

    if (!i) {
        return;
    }

    struct editor *e = _get_or_create_editor(obj);
    e->type = type;
    e->obj = obj;

    struct ct_world *w = ce_cdb_a0->write_begin(e->context_obj);
    ce_cdb_a0->set_ref(w, ASSET_EDITOR_OBJ, obj);
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

static void on_asset_double_click(uint64_t type,
                                  void *event) {
    struct ebus_cdb_event *ev = event;

    uint64_t asset_type = ce_cdb_a0->read_uint64(ev->obj,
                                                 ASSET_TYPE, 0);
    uint64_t asset_name = ce_cdb_a0->read_uint64(ev->obj,
                                                 ASSET_NAME, 0);

    struct ct_resource_id rid = {.name = asset_name, .type = asset_type};

    uint64_t obj = ct_resource_a0->get(rid);

    open(obj);
}


static struct ct_editor_module_i0 ct_editor_module_i0 = {
        .update = update,
};

static uint64_t cdb_type() {
    return _ASSET_EDITOR;
};


static struct ct_dock_i0 dock_i = {
        .cdb_type = cdb_type,
        .display_title = dock_title,
        .name = name,
        .draw_ui = draw_editor,
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


    ce_api_a0->register_api(DOCK_INTERFACE_NAME, &dock_i);


    _get_or_create_editor(0);
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
