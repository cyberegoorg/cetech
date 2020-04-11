#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory/memory.h>
#include <celib/id.h>
#include <celib/api.h>
#include <celib/cdb.h>
#include <cetech/ui/icons_font_awesome.h>
#include <cetech/renderer/gfx.h>
#include <cetech/asset/asset.h>

#include <cetech/explorer/explorer.h>
#include <stdio.h>
#include <cetech/node_graph_editor/node_graph_editor.h>
#include <cetech/asset_preview/asset_preview.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/ui/ui.h>

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
    return CT_NODE_GRAPH_ASSET;
}


void draw_raw(uint64_t obj,
              ce_vec2_t size) {
    ct_node_graph_editor_a0->draw_ng_editor(obj, 0);
}

static struct ct_asset_preview_i0 asset_preview_i0 = {
        .draw_raw = draw_raw,
};


static const char *name() {
    return "node_graph";
}


static struct ct_asset_i0 ct_asset_api = {
        .name = name,
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .create_new = create_new,
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

    ct_ui_a0->columns(selected_obj, 2, true);
    ct_ui_a0->next_column();
    ct_ui_a0->next_column();

    //

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                  top_level_obj);

    enum ct_ui_tree_node_flag flags = 0 |
                                      CT_TREE_NODE_FLAGS_OpenOnArrow |
                                      //                               DebugUITreeNodeFlags_OpenOnDoubleClick |
                                      //                               DebugUITreeNodeFlags_DefaultOpen;
                                      CT_TREE_NODE_FLAGS_CollapsingHeader |
                                      0;
    uint64_t new_selected_object = 0;


    uint64_t nodes = ce_cdb_a0->read_subobject(reader, CT_NODE_GRAPH_NODES, 0);
    const ce_cdb_obj_o0 *ns_reader = ce_cdb_a0->read(ce_cdb_a0->db(), nodes);

    const uint64_t ns_n = ce_cdb_a0->prop_count(ns_reader);

    if (!ns_n) {
        flags |= CT_TREE_NODE_FLAGS_Leaf;
    }

    char name[128] = {0};
    uint64_t uuid = top_level_obj;
    snprintf(name, CE_ARRAY_LEN(name), "Nodes");

    bool selected = selected_obj == nodes;
    if (selected) {
        flags |= CT_TREE_NODE_FLAGS_Selected;
    }

    char label[128] = {0};


    snprintf(label, CE_ARRAY_LEN(label), (ICON_FA_CUBE" %s"), name);
    const bool open = ct_ui_a0->tree_node_ex(
            &(ct_ui_tree_node_ex_t0) {
                    .id=selected_obj,
                    .text=label,
                    .flags=flags
            });

    if (ct_ui_a0->is_item_clicked(0)) {
        new_selected_object = nodes;
    }

    ct_ui_a0->next_column();

    //menu
    snprintf(label, CE_ARRAY_LEN(label), ICON_FA_PLUS
            "##add_%llu", nodes);

    bool add = ct_ui_a0->button(&(ct_ui_button_t0) {.text=label});

    uint64_t modal_id = ct_ui_a0->generate_id();
    ct_node_graph_editor_a0->add_node_modal(modal_id, uuid);

    if (add) {
        ct_ui_a0->modal_popup_open(modal_id);
    }
    ct_ui_a0->next_column();

    if (open) {
        const uint64_t *ns = ce_cdb_a0->prop_keys(ns_reader);
        for (int i = 0; i < ns_n; ++i) {
            uint64_t node = ns[i];
            snprintf(label, CE_ARRAY_LEN(label),
                     (ICON_FA_CUBE
                             " ""%llx##node%llu"), node, node);

            flags = CT_TREE_NODE_FLAGS_Leaf;

            if (selected_obj == node) {
                flags |= CT_TREE_NODE_FLAGS_Selected;
            }

            if (ct_ui_a0->tree_node_ex(
                    &(ct_ui_tree_node_ex_t0) {.id=selected_obj, .text=label, .flags=flags})) {
                if (ct_ui_a0->is_item_clicked(0)) {
                    new_selected_object = node;
                }
                ct_ui_a0->tree_pop();
            }
        }

        ct_ui_a0->tree_pop();
    }

    ct_ui_a0->next_column();
    ct_ui_a0->next_column();

    ct_ui_a0->columns(modal_id, 1, true);

    return new_selected_object;
}


static struct ct_node_i0 *get_interface(uint64_t type) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_NODE_I0);

    while (it.api) {
        struct ct_node_i0 *i = (it.api);

        if (i && i->type && (i->type() == type)) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}


static void _node_property_draw(ce_cdb_t0 db,
                                uint64_t obj,
                                uint64_t context) {
    const ce_cdb_obj_o0 *node_r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t node_type = ce_cdb_a0->read_uint64(node_r, CT_NODE_TYPE, 0);

    ct_node_i0 *ni = get_interface(node_type);

    if (!ni) {
        return;
    }


    bool open = ct_editor_ui_a0->ui_prop_header("Inputs", obj);

    if (open) {
        uint64_t inputs_o = ce_cdb_a0->read_subobject(node_r, CT_NODE_GRAPH_NODE_INPUTS, 0);
        uint32_t in_n = 0;
        const struct ct_node_pin_def *in_defs = ni->input_defs(&in_n);

        for (int i = 0; i < in_n; ++i) {
            const struct ct_node_pin_def *def = &in_defs[i];

            switch (def->type) {
                case CT_NODE_PIN_NONE:
                    break;

                case CT_NODE_PIN_FLOAT:
                    ct_editor_ui_a0->prop_float(inputs_o, def->name, def->prop, (ui_float_p0) {});
                    break;

                case CT_NODE_PIN_STRING:
                    ct_editor_ui_a0->prop_str(inputs_o, def->name, def->prop, i);
                    break;

                case CT_NODE_PIN_BOOL:
                    ct_editor_ui_a0->prop_bool(inputs_o, def->name, def->prop);
                    break;
            }
        }
    }
    ct_editor_ui_a0->ui_prop_header_end(open);
}

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


    api->add_api(CT_NODE_GRAPH_A0_STR, &ng_api, sizeof(ng_api));
    api->add_impl(CT_ASSET_I0_STR, &ct_asset_api, sizeof(ct_asset_api));

    ce_cdb_a0->reg_obj_type(CT_NODE_GRAPH_NODE, node_prop, CE_ARRAY_LEN(node_prop));
    ce_cdb_a0->set_aspect(CT_NODE_GRAPH_NODE, CT_PROPERTY_EDITOR_ASPECT, _node_property_draw);
    ce_cdb_a0->set_aspect(CT_NODE_GRAPH_NODE, CT_PREVIEW_ASPECT, &asset_preview_i0);
    ce_cdb_a0->set_aspect(CT_NODE_GRAPH_NODE, CT_EXPLORER_ASPECT, draw_ui);

}

void CE_MODULE_UNLOAD (node_graph)(struct ce_api_a0 *api,
                                   int reload) {

    CE_UNUSED(api);
}