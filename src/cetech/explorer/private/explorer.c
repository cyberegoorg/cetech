#include <string.h>

#include <celib/id.h>
#include <celib/config.h>
#include <celib/memory/memory.h>
#include <celib/api.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/yaml_cdb.h>

#include <celib/macros.h>


#include <cetech/renderer/gfx.h>

#include <cetech/editor/dock.h>
#include <cetech/explorer/explorer.h>
#include <cetech/asset/asset.h>

#include <cetech/ui/icons_font_awesome.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/ui/ui.h>

#define WINDOW_NAME "Explorer"

#define _G explorer_globals
typedef const uint64_t i1;
static struct _G {
    bool visible;
    ce_alloc_t0 *allocator;
} _G;

static ct_explorer_draw_ui_t *_get_explorer_by_type(uint64_t type) {
    return ce_cdb_a0->get_aspect(type, CT_EXPLORER_ASPECT);
}

bool _is_leaf(ce_cdb_t0 db,
              uint64_t obj) {
    const ce_cdb_obj_o0 *rs_reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    const uint64_t n = ce_cdb_a0->prop_count(rs_reader);
    const uint64_t *keys = ce_cdb_a0->prop_keys(rs_reader);

    for (int i = 0; i < n; ++i) {
        uint64_t prop = keys[i];
        ce_cdb_type_e0 type = ce_cdb_a0->prop_type(rs_reader, prop);

        if ((type == CE_CDB_TYPE_SUBOBJECT)
            || (type == CE_CDB_TYPE_SET_SUBOBJECT)) {
            return false;
        }
    }

    return true;
}

uint64_t draw_ui_generic(uint64_t obj,
                         uint64_t selected_object,
                         uint64_t context) {
    const ce_cdb_obj_o0 *rs_reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    const uint64_t n = ce_cdb_a0->prop_count(rs_reader);
    const uint64_t *keys = ce_cdb_a0->prop_keys(rs_reader);

    uint64_t new_selected = 0;

    for (uint32_t i = 0; i < n; ++i) {
        ce_cdb_type_e0 type = ce_cdb_a0->prop_type(rs_reader, keys[i]);

        const char *text = ce_id_a0->str_from_id64(keys[i]);

        if (type == CE_CDB_TYPE_SUBOBJECT) {
            uint64_t sub_obj = ce_cdb_a0->read_subobject(rs_reader, keys[i], 0);

            bool is_lef = _is_leaf(ce_cdb_a0->db(), sub_obj);
            enum ct_ui_tree_node_flag flag = is_lef ? CT_TREE_NODE_FLAGS_Leaf : 0;


            const bool open = ct_ui_a0->tree_node_ex(&(ct_ui_tree_node_ex_t0) {
                    .id=obj,
                    .text=text,
                    .flags = flag,
            });


            if (ct_ui_a0->is_item_clicked(0)) {
                new_selected = sub_obj;
            }

            if (open) {
                uint64_t new_obj = draw_ui_generic(sub_obj, selected_object, context);
                if (new_obj) {
                    new_selected = new_obj;
                }

                ct_ui_a0->tree_pop();
            }
        } else if (type == CE_CDB_TYPE_SET_SUBOBJECT) {
            const bool open = ct_ui_a0->tree_node_ex(&(ct_ui_tree_node_ex_t0) {
                    .id=obj,
                    .text=text});
            if (open) {
                uint64_t obj_set_num = ce_cdb_a0->read_objset_num(rs_reader, keys[i]);
                uint64_t obj_set[obj_set_num];
                ce_cdb_a0->read_objset(rs_reader, keys[i], obj_set);

                for (int j = 0; j < obj_set_num; ++j) {
                    uint64_t sub_obj = obj_set[j];
                    uint64_t sub_obj_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), sub_obj);

                    bool is_lef = _is_leaf(ce_cdb_a0->db(), sub_obj);
                    enum ct_ui_tree_node_flag flag = is_lef ? CT_TREE_NODE_FLAGS_Leaf : 0;

                    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), sub_obj);
                    const char *name = ce_cdb_a0->read_str(r, ce_id_a0->id64("name"), NULL);

                    const bool sub_open = ct_ui_a0->tree_node_ex(
                            &(ct_ui_tree_node_ex_t0) {
                                    .id=sub_obj,
                                    .text= name ? name : ce_id_a0->str_from_id64(sub_obj_type),
                                    .flags = flag});

                    if (ct_ui_a0->is_item_clicked(0)) {
                        new_selected = sub_obj;
                    }

                    if (sub_open) {
                        u_int64_t new_obj = draw_ui_generic(obj_set[j],
                                                            selected_object,
                                                            context);
                        if (new_obj) {
                            new_selected = new_obj;
                        }
                        ct_ui_a0->tree_pop();
                    }
                }
                ct_ui_a0->tree_pop();
            }
        }
    }

    return new_selected;
}

static uint64_t draw(ce_cdb_t0 db,
                     uint64_t selected_obj,
                     uint64_t context) {
    const uint64_t root_obj = ce_cdb_a0->find_root(db, selected_obj);
    const uint64_t root_obj_type = ce_cdb_a0->obj_type(db, root_obj);
    ct_explorer_draw_ui_t *draw_ui = _get_explorer_by_type(root_obj_type);

    if (draw_ui) {
        return draw_ui(root_obj, selected_obj, context);
    }

    bool is_lef = _is_leaf(db, root_obj);

    enum ct_ui_tree_node_flag flag = (is_lef ? CT_TREE_NODE_FLAGS_Leaf : 0)
                                     | (!is_lef ? CT_TREE_NODE_FLAGS_DefaultOpen : 0);


    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, root_obj);
    const char *name = ce_cdb_a0->read_str(r, ce_id_a0->id64("name"), NULL);
    const char *text = name ? name : ce_id_a0->str_from_id64(ce_cdb_a0->obj_type(db, root_obj));

    const bool open = ct_ui_a0->tree_node_ex(&(ct_ui_tree_node_ex_t0) {
            .id=root_obj,
            .text= text,
            .flags = flag,
    });

    uint64_t new_selected = 0;

    if (ct_ui_a0->is_item_clicked(0)) {
        new_selected = root_obj;
    }

    if (open) {
        new_selected = draw_ui_generic(root_obj, selected_obj, context);
        ct_ui_a0->tree_pop();
    }

    return new_selected;
}

static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    if (!selected_object) {
        return;
    }

    ct_ui_a0->separator();

    uint64_t new_selected_object = draw(ce_cdb_a0->db(), selected_object, context);
    if (new_selected_object) {
        ct_selected_object_a0->set_selected_object(context, new_selected_object);
    }
}


static const char *dock_title() {
    return ICON_FA_TREE " " WINDOW_NAME;
}

static const char *name(uint64_t dock) {
    return "explorer";
}

static uint64_t dock_flags() {
    return 0;
}

static struct ct_dock_i0 dock_api = {
        .type = CT_EXPLORER_ASPECT,
        .ui_flags = dock_flags,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
};

void CE_MODULE_LOAD(level_inspector)(struct ce_api_a0 *api,
                                     int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_asset_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .visible = true
    };

    api->add_impl(CT_DOCK_I0_STR, &dock_api, sizeof(dock_api));

    ct_dock_a0->create_dock(CT_EXPLORER_ASPECT, true);
}

void CE_MODULE_UNLOAD(level_inspector)(struct ce_api_a0 *api,
                                       int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _G = (struct _G) {};
}
