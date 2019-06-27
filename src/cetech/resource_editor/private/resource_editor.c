#include <string.h>

#include <celib/memory/allocator.h>
#include <celib/cdb.h>
#include <celib/ydb.h>
#include <celib/macros.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"


#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/editor/dock.h>
#include <celib/containers/array.h>
#include <cetech/resource_editor/resource_editor.h>
#include <cetech/editor/selcted_object.h>

#define _G editor_globals

#define RESOURCE_EDITOR\
    CE_ID64_0("asset_editor", 0xd7ecca607e454ce9ULL)


typedef struct editor {
    char title[128];
    uint64_t type;
    uint64_t context_obj;

    uint64_t top_level;
    uint64_t selected_object;
//    bool locked;
} editor;

static struct _G {
    editor *editors;
} _G;

static struct editor *_get_or_create_editor() {
    int idx = ce_array_size(_G.editors);
    ce_array_push(_G.editors, (editor) {}, ce_memory_a0->system);
    editor *ed = &_G.editors[idx];
    return ed;
}


static struct ct_resource_editor_i0 *get_resource_editor(uint64_t cdb_type) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_RESOURCE_EDITOR_I);
    while (it.api) {
        struct ct_resource_editor_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
};

static void set_asset(editor *ei,
                      uint64_t obj) {
    if (!ei) {
        return;
    }

//    if (ei->locked) {
//        return;
//    }

    if (ei->selected_object == obj) {
        return;
    }

    uint64_t top_level = ce_cdb_a0->find_root(ce_cdb_a0->db(), obj);

    if (ei->top_level == top_level) {
        return;
    }

    if (ei->top_level) {
        ct_resource_editor_i0 *i = get_resource_editor(ei->type);

        if (!i) {
            return;
        }

        if (i->close) {
            i->close(ei->context_obj);
        }

        ei->type = 0;
        ei->top_level = 0;
    }

    ei->top_level = 0;
    ei->selected_object = obj;

    if (obj) {
        uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);
        ei->type = type;

        ct_resource_editor_i0 *i = get_resource_editor(ei->type);

        if (!i) {
            return;
        }

        if (i->open) {
            ei->context_obj = i->open(obj);
        }
        ei->top_level = top_level;
    }
}


static void draw_editor(uint64_t content,
                        uint64_t context,
                        uint64_t selected_object) {
    editor *editor = (struct editor *) content;

    if (!editor) {
        return;
    }

    set_asset(editor, ct_selected_object_a0->selected_object(context));

    ct_resource_editor_i0 *i = get_resource_editor(editor->type);

    if (!i) {
        return;
    }

    bool is_mouse_hovering = ct_debugui_a0->IsMouseHoveringWindow();
    bool click = ct_debugui_a0->IsMouseClicked(0, false);

    if (is_mouse_hovering && click) {
        uint64_t obj = editor->selected_object;

        ct_selected_object_a0->set_selected_object(context, obj);
    }

    i->draw_ui(editor->context_obj, context);
}

static void draw_editor_menu(uint64_t content,
                             uint64_t context,
                             uint64_t selected_object) {
    editor *editor = (struct editor *) content;

    if (!editor) {
        return;
    }

    ct_resource_editor_i0 *i = get_resource_editor(editor->type);

    if (!i) {
        return;
    }

//    uint64_t locked_object = 0; //ct_editor_ui_a0->lock_selected_obj(dock, editor->selected_object);
//
//    editor->locked = false;
//
//    if (locked_object) {
//        editor->selected_object = locked_object;
//        editor->locked = true;
//    }

    if (i && i->draw_menu) {
        ct_debugui_a0->SameLine(0, -1);
        i->draw_menu(editor->context_obj);
    }
}

//static uint32_t find_editor(uint64_t obj) {
//    const uint32_t editor_n = ce_array_size(_G.editors);
//
//    for (uint32_t i = 0; i < editor_n; ++i) {
//        struct editor *editor = &_G.editors[i];
//
//        if (editor->obj == obj) {
//            return i;
//        }
//    }
//
//    return UINT32_MAX;
//}

#define DEFAULT_EDITOR_NAME  "Editor"

static const char *dock_title(uint64_t content,
                              uint64_t selected_object) {
    return DEFAULT_EDITOR_NAME;
}

static const char *name(uint64_t dock) {
    return "editor";
}

static uint64_t open() {
    return (uint64_t) _get_or_create_editor();
}

static void update(float dt) {
    const uint32_t editor_n = ce_array_size(_G.editors);
    for (uint8_t i = 0; i < editor_n; ++i) {
        struct editor *editor = &_G.editors[i];

        struct ct_resource_editor_i0 *editor_i = get_resource_editor(editor->type);

        if (!editor_i) {
            continue;
        }

        editor_i->update(editor->context_obj, dt);
    }
}


static struct ct_editor_module_i0 editor_module_api = {
        .update = update,
};

static uint64_t dock_flags() {
    return DebugUIWindowFlags_NoNavInputs |
           DebugUIWindowFlags_NoScrollbar |
           DebugUIWindowFlags_NoScrollWithMouse;
}


static struct ct_dock_i0 dock_i = {
        .type = RESOURCE_EDITOR,
        .ui_flags = dock_flags,
        .display_title = dock_title,
        .name = name,
        .draw_ui = draw_editor,
        .draw_menu = draw_editor_menu,
        .open = open,
};


void CE_MODULE_LOAD(editor)(struct ce_api_a0 *api,
                            int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ct_camera_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_rg_a0);
    CE_INIT_API(api, ct_default_rg_a0);

    _G = (struct _G) {
    };

    ce_api_a0->add_impl(CT_DOCK_I, &dock_i, sizeof(dock_i));
    ce_api_a0->add_impl(CT_EDITOR_MODULE_I, &editor_module_api, sizeof(editor_module_api));

    ct_dock_a0->create_dock(RESOURCE_EDITOR, true);
}

void CE_MODULE_UNLOAD(editor)(struct ce_api_a0 *api,
                              int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _G = (struct _G) {};
}

