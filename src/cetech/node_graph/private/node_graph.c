#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory/memory.h>
#include <celib/id.h>
#include <celib/api.h>
#include <celib/cdb.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/renderer/gfx.h>
#include <cetech/resource/resource.h>
#include <cetech/debugui/debugui.h>
#include <cetech/explorer/explorer.h>
#include <stdio.h>
#include <cetech/node_graph_editor/node_graph_editor.h>
#include <cetech/resource/resource_preview.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/editor/editor_ui.h>

#include "../node_graph.h"

#define _G node_graph_globals
static struct _G {
    ce_alloc_t0 *allocator;
} _G;

static void create_new(uint64_t obj) {
    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);

    if (!ce_cdb_a0->prop_exist(w, CT_NODE_GRAPH_NODES)) {
        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                               CT_NODE_GRAPH_NODES);
        ce_cdb_a0->set_subobject(w, CT_NODE_GRAPH_NODES, ch);
    }

    if (!ce_cdb_a0->prop_exist(w, CT_NODE_GRAPH_CONNECTIONS)) {
        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                               CT_NODE_GRAPH_CONNECTIONS);
        ce_cdb_a0->set_subobject(w, CT_NODE_GRAPH_CONNECTIONS, ch);
    }

    ce_cdb_a0->write_commit(w);
}

static const char *display_icon() {
    return ICON_FA_CUBES;
}

static uint64_t cdb_type() {
    return CT_NODE_GRAPH_RESOURCE;
}


void draw_raw(uint64_t obj,
              ce_vec2_t size) {
    ct_node_graph_editor_a0->draw_ng_editor(obj, 0);
}

static struct ct_resource_preview_i0 resource_preview_i0 = {
        .draw_raw = draw_raw,
};

static void *get_res_interface(uint64_t name_hash) {
    if (name_hash == RESOURCE_PREVIEW_I) {
        return &resource_preview_i0;
    }
    return NULL;
}

static struct ct_resource_i0 ct_resource_api = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .create_new = create_new,
        .get_interface= get_res_interface,
};


static uint64_t draw_ui(uint64_t top_level_obj,
                        uint64_t selected_obj,
                        uint64_t context) {
    if (!top_level_obj) {
        return 0;
    }

    if (!selected_obj) {
        return 0;
    }

    ct_debugui_a0->Columns(2, NULL, true);
    ct_debugui_a0->NextColumn();
    ct_debugui_a0->NextColumn();


    //

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                  top_level_obj);

    ImGuiTreeNodeFlags flags = 0 |
                               DebugUITreeNodeFlags_OpenOnArrow |
                               //                               DebugUITreeNodeFlags_OpenOnDoubleClick |
                               //                               DebugUITreeNodeFlags_DefaultOpen;
                               DebugUITreeNodeFlags_CollapsingHeader |
                               0;
    uint64_t new_selected_object = 0;


    uint64_t nodes = ce_cdb_a0->read_subobject(reader, CT_NODE_GRAPH_NODES, 0);
    const ce_cdb_obj_o0 *ns_reader = ce_cdb_a0->read(ce_cdb_a0->db(), nodes);

    const uint64_t ns_n = ce_cdb_a0->prop_count(ns_reader);

    if (!ns_n) {
        flags |= DebugUITreeNodeFlags_Leaf;
    }

    char name[128] = {0};
    uint64_t uid = top_level_obj;
    snprintf(name, CE_ARRAY_LEN(name), "Nodes");

    bool selected = selected_obj == nodes;
    if (selected) {
        flags |= DebugUITreeNodeFlags_Selected;
    }

    char label[128] = {0};


    snprintf(label, CE_ARRAY_LEN(label),
             (ICON_FA_CUBE
                     " ""%s##nodes_%llu"), name, uid);
    const bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        new_selected_object = nodes;
    }

    ct_debugui_a0->NextColumn();

    //menu
    snprintf(label, CE_ARRAY_LEN(label), ICON_FA_PLUS
            "##add_%llu", nodes);

    bool add = ct_debugui_a0->Button(label, &CE_VEC2_ZERO);

    char modal_id[128] = {'\0'};
    sprintf(modal_id, "select...##select_node_%llu", uid);
    ct_node_graph_editor_a0->add_node_modal(modal_id, uid);

    if (add) {
        ct_debugui_a0->OpenPopup(modal_id);
    }
    ct_debugui_a0->NextColumn();

    if (open) {
        const uint64_t *ns = ce_cdb_a0->prop_keys(ns_reader);
        for (int i = 0; i < ns_n; ++i) {
            uint64_t node = ns[i];
            snprintf(label, CE_ARRAY_LEN(label),
                     (ICON_FA_CUBE
                             " ""%llx##node%llu"), node, node);

            flags = DebugUITreeNodeFlags_Leaf;

            if (selected_obj == node) {
                flags |= DebugUITreeNodeFlags_Selected;
            }

            if (ct_debugui_a0->TreeNodeEx(label, flags)) {
                if (ct_debugui_a0->IsItemClicked(0)) {
                    new_selected_object = node;
                }
                ct_debugui_a0->TreePop();
            }
        }

        ct_debugui_a0->TreePop();
    }

    ct_debugui_a0->NextColumn();
    ct_debugui_a0->NextColumn();

    ct_debugui_a0->Columns(1, NULL, true);

    return new_selected_object;
}

static void draw_menu(uint64_t selected_obj,
                      uint64_t context) {
    if (!selected_obj) {
        return;
    }

}

static struct ct_node_i0 *get_interface(uint64_t type) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_NODE_I);

    while (it.api) {
        struct ct_node_i0 *i = (it.api);

        if (i && i->type && (i->type() == type)) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}


static uint64_t _node_property_cdb_type() {
    return CT_NODE_GRAPH_NODE;
}

static void _node_property_draw(uint64_t obj,
                                uint64_t context) {
    const ce_cdb_obj_o0 *node_r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t node_type = ce_cdb_a0->read_uint64(node_r, CT_NODE_TYPE, 0);

    ct_node_i0 *ni = get_interface(node_type);

    if (!ni) {
        return;
    }


    ct_editor_ui_a0->ui_prop_header("Inputs");
    ct_debugui_a0->Separator();


    uint64_t inputs_o = ce_cdb_a0->read_subobject(node_r,
                                                  CT_NODE_GRAPH_NODE_INPUTS,
                                                  0);

    uint32_t in_n = 0;
    const struct ct_node_pin_def *in_defs = ni->input_defs(&in_n);

    for (int i = 0; i < in_n; ++i) {
        const struct ct_node_pin_def *def = &in_defs[i];

        switch (def->type) {
            case CT_NODE_PIN_NONE:
                break;

            case CT_NODE_PIN_FLOAT:
                ct_editor_ui_a0->prop_float(inputs_o,
                                            def->name, def->prop,
                                            (ui_float_p0) {});
                break;

            case CT_NODE_PIN_STRING:
                ct_editor_ui_a0->prop_str(inputs_o,
                                          def->name, def->prop, i);
                break;

            case CT_NODE_PIN_BOOL:
                ct_editor_ui_a0->prop_bool(inputs_o,
                                           def->name, def->prop);
                break;
        }
    }
}

static struct ct_property_editor_i0 node_property_editor_i0 = {
        .cdb_type = _node_property_cdb_type,
        .draw_ui = _node_property_draw,
};

static struct ct_node_graph_a0 ng_api = {
        .get_interface = get_interface,
};

struct ct_node_graph_a0 *ct_node_graph_a0 = &ng_api;

typedef struct _node_obj_t {
    uint64_t inputs;
    uint64_t outputs;
} _node_obj_t;

static ce_cdb_prop_def_t0 node_prop[] = {
        {.name = "inputs", .type = CE_CDB_TYPE_SUBOBJECT, .obj_type = CT_NODE_GRAPH_NODE_INPUTS},
        {.name = "outputs", .type = CE_CDB_TYPE_SUBOBJECT, .obj_type = CT_NODE_GRAPH_NODE_OUTPUTS},
};

void CE_MODULE_LOAD (node_graph)(struct ce_api_a0 *api,
                                 int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_id_a0->id64("node_graph");
    ce_id_a0->id64("nodes");
    ce_id_a0->id64("connections");
    ce_id_a0->id64("position_x");
    ce_id_a0->id64("position_y");
    ce_id_a0->id64("size_x");
    ce_id_a0->id64("size_y");
    ce_id_a0->id64("inputs");
    ce_id_a0->id64("outputs");
    ce_id_a0->id64("from");
    ce_id_a0->id64("from_pin");
    ce_id_a0->id64("to");
    ce_id_a0->id64("to_pin");


    static struct ct_explorer_i0 entity_explorer = {
            .cdb_type = cdb_type,
            .draw_ui = draw_ui,
            .draw_menu = draw_menu,
    };

    api->register_api(CT_NODE_GRAPH_API, &ng_api, sizeof(ng_api));
    api->register_api(RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));
    api->register_api(EXPLORER_INTERFACE, &entity_explorer, sizeof(entity_explorer));
    api->register_api(CT_PROPERTY_EDITOR_INTERFACE, &node_property_editor_i0,
                      sizeof(node_property_editor_i0));

    ce_cdb_a0->reg_obj_type(CT_NODE_GRAPH_NODE, node_prop, CE_ARRAY_LEN(node_prop));

}

void CE_MODULE_UNLOAD (node_graph)(struct ce_api_a0 *api,
                                   int reload) {

    CE_UNUSED(api);
}