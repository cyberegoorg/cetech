#include <math.h>

#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory/memory.h>
#include <celib/id.h>
#include <celib/api.h>

extern "C" {
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/node_graph/node_graph.h>
#include <cetech/asset_editor/asset_editor.h>
#include <cetech/renderer/gfx.h>

#include "../node_graph_editor.h"
};


#include <cetech/debugui/private/ocornut-imgui/imgui.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include "cetech/debugui/private/ocornut-imgui/imgui_internal.h"

#include <cetech/debugui/debugui.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/ecs/ecs.h>
#include <fnmatch.h>
#include <cetech/editor/selcted_object.h>
#include <celib/math/math.h>


#define _G node_graph_editor_globals
static struct _G {
    ce_alloc_t0 *allocator;
} _G;


static void close(uint64_t context_obj) {

}

static uint64_t open(uint64_t obj) {
    return 0;
}

static void update(uint64_t context_obj,
                   float dt) {

}


static uint64_t _new_node(uint64_t graph,
                          uint64_t type) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), graph);

    uint64_t node = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                             CT_NODE_GRAPH_NODE);

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), node);
    ce_cdb_a0->set_uint64(w, CT_NODE_TYPE, type);
    ce_cdb_a0->write_commit(w);

    uint64_t nodes = ce_cdb_a0->read_subobject(reader, CT_NODE_GRAPH_NODES, 0);
    w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), nodes);
    ce_cdb_a0->set_subobject(w, node, node);
    ce_cdb_a0->write_commit(w);

    return node;
}

static char modal_buffer[128] = {};

static void add_node_modal(const char *modal_id,
                           uint64_t obj) {


    bool open = true;
    ce_vec2_t size = {512, 512};
    ct_debugui_a0->SetNextWindowSize(&size,
                                     static_cast<DebugUICond>(0));
    if (ct_debugui_a0->BeginPopupModal(modal_id, &open, 0)) {
        struct ct_controler_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_debugui_a0->CloseCurrentPopup();
            ct_debugui_a0->EndPopup();
            return;
        }

        char labelidi[128] = {'\0'};
        sprintf(labelidi, "##modal_node_add_input%llu", obj);

        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer,
                                 CE_ARRAY_LEN(modal_buffer),
                                 0,
                                 0, NULL);


        struct ce_api_entry_t0 it = ce_api_a0->first(CT_NODE_I0);
        while (it.api) {
            struct ct_node_i0 *i = static_cast<ct_node_i0 *>(it.api);
            struct ct_editor_node_i0 *ei;

            if (!i->get_interface) {
                it = ce_api_a0->next(it);
                continue;
            }

            ei = static_cast<ct_editor_node_i0 *>(i->get_interface(
                    CT_EDITOR_NODE_I0));

            if (ei->display_name) {
                const char *label = ei->display_name();

                if (modal_buffer[0]) {
                    char filter[256] = {};
                    snprintf(filter, CE_ARRAY_LEN(filter),
                             "*%s*", modal_buffer);

                    if (0 != fnmatch(filter, label, FNM_CASEFOLD)) {
                        it = ce_api_a0->next(it);
                        continue;
                    }
                }


                bool add = ct_debugui_a0->Selectable(label, false, 0,
                                                     &CE_VEC2_ZERO);

                if (add) {
                    _new_node(obj, i->type());
                    modal_buffer[0] = '\0';
                }
            }

            it = ce_api_a0->next(it);
        }

        ct_debugui_a0->EndPopup();
    }
}

ce_vec2_t _get_node_in_pin_pos(ce_vec2_t Pos,
                               ce_vec2_t Size,
                               uint32_t pin_n,
                               int pin_no) {
    return (ce_vec2_t) {
            .x = Pos.x,
            .y = Pos.y + Size.y * ((float) pin_no + 1) / ((float) pin_n + 1),
    };
}

ce_vec2_t _get_node_out_pin_pos(ce_vec2_t Pos,
                                ce_vec2_t Size,
                                uint32_t pin_n,
                                int pin_no) {
    return (ce_vec2_t) {
            .x = Pos.x + Size.x,
            .y = Pos.y + Size.y * ((float) pin_no + 1) / ((float) pin_n + 1),
    };
}

// TODO: SHIT move to cdb => dock context
static struct drag_node_s {
    uint64_t from_obj;
    uint32_t pin_idx;
    bool input;
    ce_vec2_t size;
    ce_vec2_t pos;
    uint32_t count;
    uint64_t pin_name;
    ct_node_pin_def pin_def;
} drag_node = {};

static void _add_link(uint64_t graph,
                      uint64_t output,
                      uint64_t output_pin,
                      uint64_t input,
                      uint64_t input_pin) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), graph);

    uint64_t conns = ce_cdb_a0->read_subobject(reader,
                                               CT_NODE_GRAPH_CONNECTIONS,
                                               0);
    if (conns) {
        uint64_t conn_obj = ce_cdb_a0->create_object(
                ce_cdb_a0->db(), 0);

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(
                ce_cdb_a0->db(), conn_obj);
        ce_cdb_a0->set_ref(w, CT_NODE_GRAPH_CONN_FROM, output);
        ce_cdb_a0->set_uint64(w, CT_NODE_GRAPH_CONN_FROM_PIN, output_pin);

        ce_cdb_a0->set_ref(w, CT_NODE_GRAPH_CONN_TO, input);
        ce_cdb_a0->set_uint64(w, CT_NODE_GRAPH_CONN_TO_PIN, input_pin);
        ce_cdb_a0->write_commit(w);

        w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), conns);
        ce_cdb_a0->set_subobject(w, conn_obj, conn_obj);
        ce_cdb_a0->write_commit(w);
    }
}

const ce_vec2_t NODE_WINDOW_PADDING = {8.0f, 8.0f};

static uint32_t _pin_idx_by_name(const ct_node_pin_def *defs,
                                 uint32_t def_n,
                                 uint64_t name) {
    for (uint32_t i = 0; i < def_n; ++i) {
        const ct_node_pin_def *def = &defs[i];
        if (ce_id_a0->id64(def->name) != name) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

static ct_node_i0 *_get_node_i(const ce_cdb_obj_o0 *reader) {
    uint64_t node_type = ce_cdb_a0->read_uint64(reader, CT_NODE_TYPE, 0);
    ct_node_i0 *ni = ct_node_graph_a0->get_interface(node_type);

    return ni;
}


#define _to_imvec(v) (ImVec2(v.x, v.y))
#define _to_vec2(v) ((ce_vec2_t){.x = v.x, .y = v.y})

static void _display_grid(ce_vec2_t scrolling,
                          ImDrawList *draw_list) {
    ImU32 GRID_COLOR = IM_COL32(0, 200, 0, 40);
    float GRID_SZ = 64.0f;
    ImVec2 win_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetWindowSize();
    for (float x = fmodf(scrolling.x, GRID_SZ);
         x < canvas_sz.x; x += GRID_SZ)
        draw_list->AddLine(ImVec2(x, 0.0f) + win_pos,
                           ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
    for (float y = fmodf(scrolling.y, GRID_SZ);
         y < canvas_sz.y; y += GRID_SZ)
        draw_list->AddLine(ImVec2(0.0f, y) + win_pos,
                           ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
}

static ImU32 _pin_col[] = {
        [CT_NODE_PIN_NONE] = IM_COL32(255, 255, 255, 255),
        [CT_NODE_PIN_FLOAT] = IM_COL32(0, 0, 175, 255),
        [CT_NODE_PIN_STRING] = IM_COL32(175, 0, 0, 255),
        [CT_NODE_PIN_BOOL] = IM_COL32(0, 175, 0, 255),
};

static void _draw_connection(const ct_node_pin_def *def,
                             ce_vec2_t p1,
                             ce_vec2_t p2,
                             ImDrawList *draw_list) {
    ImU32 col = _pin_col[def->type];

    draw_list->AddLine(*(ImVec2 *) &p1,
                       *(ImVec2 *) &p2, col, 3.0f);
}

static void _draw_dgraged_line(ce_vec2_t offset,
                               ImDrawList *draw_list) {
    ImGuiIO &io = ImGui::GetIO();

    ce_vec2_t p2;
    const ce_vec2_t p1 = {
            .x = io.MousePos.x,
            .y = io.MousePos.y,
    };

    if (!drag_node.input) {
        p2 = ce_vec2_add(offset, _get_node_out_pin_pos(drag_node.pos,
                                                       drag_node.size,
                                                       drag_node.count,
                                                       drag_node.pin_idx));
    } else {
        p2 = ce_vec2_add(offset, _get_node_in_pin_pos(drag_node.pos,
                                                      drag_node.size,
                                                      drag_node.count,
                                                      drag_node.pin_idx));
    }

    _draw_connection(&drag_node.pin_def, p1, p2, draw_list);
}

static void _draw_nodes(uint64_t graph,
                        ce_vec2_t offset,
                        ImDrawList *draw_list,
                        bool isLMBDraggingForMakingLinks,
                        uint64_t context) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), graph);

    uint64_t nodes = ce_cdb_a0->read_subobject(reader, CT_NODE_GRAPH_NODES, 0);
    const ce_cdb_obj_o0 *ns_reader = ce_cdb_a0->read(ce_cdb_a0->db(), nodes);
    const uint64_t *ns = ce_cdb_a0->prop_keys(ns_reader);
    const uint64_t ns_n = ce_cdb_a0->prop_count(ns_reader);
    for (int i = 0; i < ns_n; ++i) {
        uint64_t node = ns[i];


        const ce_cdb_obj_o0 *nreader = ce_cdb_a0->read(ce_cdb_a0->db(), node);

        ce_vec2_t pos = {
                ce_cdb_a0->read_float(nreader,
                                      CT_NODE_GRAPH_NODE_POS_X, 0.0f),
                ce_cdb_a0->read_float(nreader,
                                      CT_NODE_GRAPH_NODE_POS_Y, 0.0f)
        };

        ct_node_i0 *ni = _get_node_i(nreader);

        if (!ni) {
            continue;
        }

        ImGui::PushID(node);

        ct_editor_node_i0 *eni = (ct_editor_node_i0 *) ni->get_interface(
                CT_EDITOR_NODE_I0);

        uint32_t input_n = 0;
        const ct_node_pin_def *inputs = ni->input_defs(&input_n);

        uint32_t output_n = 0;
        const ct_node_pin_def *outputs = ni->output_defs(&output_n);

        const char *disply_name = eni->display_name();

        ce_vec2_t size;

        ce_vec2_t node_rect_min = ce_vec2_add(offset, pos);

        // Display node contents first
        draw_list->ChannelsSetCurrent(1); // Foreground

        ImGui::SetCursorScreenPos(
                _to_imvec(ce_vec2_add(node_rect_min, NODE_WINDOW_PADDING)));
        ImGui::BeginGroup(); // Lock horizontal position
        ImGui::Text("%s", disply_name);
        uint32_t pin_n = input_n > output_n ? input_n : output_n;
        for (int k = 0; k < pin_n; ++k) {
            ImGui::Text("");
        }
        ImGui::EndGroup();

        size = ce_vec2_add(ce_vec2_add(_to_vec2(ImGui::GetItemRectSize()),
                                       NODE_WINDOW_PADDING),
                           NODE_WINDOW_PADDING);

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), node);
        ce_cdb_a0->set_float(w, CT_NODE_GRAPH_NODE_SIZE_X, size.x);
        ce_cdb_a0->set_float(w, CT_NODE_GRAPH_NODE_SIZE_Y, size.y);
        ce_cdb_a0->write_commit(w);

        ce_vec2_t node_rect_max = ce_vec2_add(node_rect_min, size);

        draw_list->ChannelsSetCurrent(0); // Background
        ImGui::SetCursorScreenPos(_to_imvec(node_rect_min));
        ImGui::InvisibleButton("node", _to_imvec(size));
        if (ImGui::IsItemHovered()) {

        }

        bool dragged = ImGui::IsMouseDragging(0);

        bool item_active = ImGui::IsItemActive();
        if (item_active && !dragged) {
            ct_selected_object_a0->set_selected_object(context, node);
        }

        bool node_moving = item_active && dragged;
        if (node_moving) {
            pos = ce_vec2_add(pos, _to_vec2(ImGui::GetIO().MouseDelta));

            ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), node);
            ce_cdb_a0->set_float(w, CT_NODE_GRAPH_NODE_POS_X, pos.x);
            ce_cdb_a0->set_float(w, CT_NODE_GRAPH_NODE_POS_Y, pos.y);
            ce_cdb_a0->write_commit(w);

            drag_node = drag_node_s();
        }

        const float NODE_SLOT_RADIUS = 4.0f;
        const float NODE_SLOT_RADIUS_SQUARED = (NODE_SLOT_RADIUS *
                                                NODE_SLOT_RADIUS);

        ce_vec2_t NODE_SLOT_RADIUS_V = {.x = NODE_SLOT_RADIUS, .y=NODE_SLOT_RADIUS};

        uint64_t selected_obj = ct_selected_object_a0->selected_object(context);

        ImU32 node_bg_color = selected_obj == node ? IM_COL32(0, 75, 0, 255)
                                                   : IM_COL32(0, 60, 0, 255);
        draw_list->AddRectFilled(_to_imvec(node_rect_min),
                                 _to_imvec(node_rect_max), node_bg_color, 4.0f);

        draw_list->AddRect(_to_imvec(node_rect_min), _to_imvec(node_rect_max),
                           IM_COL32(0, 255, 0, 255), 4.0f);

        for (int j = 0; j < input_n; ++j) {
            const ct_node_pin_def *def = &inputs[j];
            ce_vec2_t pin_pos = ce_vec2_add(offset,
                                            _get_node_in_pin_pos(pos, size,
                                                                 input_n, j));
            const char *name = def->name;
            const char *name_end = name + strlen(name);


            draw_list->AddText(_to_imvec(pin_pos),
                               IM_COL32(0, 255, 0, 255),
                               name, name_end);

            draw_list->AddRectFilled(
                    _to_imvec(ce_vec2_sub(pin_pos, NODE_SLOT_RADIUS_V)),
                    _to_imvec(ce_vec2_add(pin_pos, NODE_SLOT_RADIUS_V)),
                    _pin_col[def->type]);

            if (!node_moving) {
                float x = ImGui::GetIO().MousePos.x - pin_pos.x;
                float y = ImGui::GetIO().MousePos.y - pin_pos.y;

                if ((x * x) + (y * y) < NODE_SLOT_RADIUS_SQUARED) {
                    if (isLMBDraggingForMakingLinks && !drag_node.from_obj) {
                        drag_node.from_obj = node;
                        drag_node.pin_idx = j;
                        drag_node.pin_name = ce_id_a0->id64(name);
                        drag_node.input = true;
                        drag_node.size = size;
                        drag_node.pos = pos;
                        drag_node.count = input_n;
                        drag_node.pin_def = *def;
                    } else if (drag_node.from_obj &&
                               drag_node.from_obj != node &&
                               !drag_node.input &&
                               !isLMBDraggingForMakingLinks &&
                               drag_node.pin_def.type == def->type) {

                        _add_link(graph,
                                  drag_node.from_obj, drag_node.pin_name,
                                  node, ce_id_a0->id64(def->name));
                    }
                }
            }
        }

        for (int j = 0; j < output_n; ++j) {
            const ct_node_pin_def *def = &outputs[j];
            ce_vec2_t pin_pos = ce_vec2_add(offset,
                                            _get_node_out_pin_pos(pos, size,
                                                                  output_n, j));

            const char *name = def->name;
            const char *name_end = name + strlen(name);


            if (!node_moving) {
                float x = ImGui::GetIO().MousePos.x - pin_pos.x;
                float y = ImGui::GetIO().MousePos.y - pin_pos.y;

                if ((x * x) + (y * y) < NODE_SLOT_RADIUS_SQUARED) {
                    if (isLMBDraggingForMakingLinks && !drag_node.from_obj) {
                        drag_node.from_obj = node;
                        drag_node.pin_idx = j;
                        drag_node.pin_name = ce_id_a0->id64(name);
                        drag_node.input = false;
                        drag_node.size = size;
                        drag_node.pos = pos;
                        drag_node.count = input_n;
                        drag_node.pin_def = *def;
                    } else if (drag_node.from_obj &&
                               drag_node.from_obj != node &&
                               drag_node.input &&
                               !isLMBDraggingForMakingLinks &&
                               drag_node.pin_def.type == def->type) {
                        _add_link(graph, node, ce_id_a0->id64(def->name),
                                  drag_node.from_obj, drag_node.pin_name);
                    }
                }
            }

            draw_list->AddText(_to_imvec(pin_pos),
                               IM_COL32(0, 255, 0, 255),
                               name, name_end);

            draw_list->AddRectFilled(
                    _to_imvec(ce_vec2_sub(pin_pos, NODE_SLOT_RADIUS_V)),
                    _to_imvec(ce_vec2_add(pin_pos, NODE_SLOT_RADIUS_V)),
                    _pin_col[def->type]);

        }


        ImGui::PopID();
    }
}

static void _draw_connections(uint64_t graph,
                              ce_vec2_t offset,
                              ImDrawList *draw_list) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), graph);


    uint64_t connections = ce_cdb_a0->read_subobject(reader,
                                                     CT_NODE_GRAPH_CONNECTIONS,
                                                     0);
    const ce_cdb_obj_o0 *cs_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                     connections);

    const uint64_t *cs = ce_cdb_a0->prop_keys(cs_reader);
    const uint64_t cs_n = ce_cdb_a0->prop_count(cs_reader);
    for (int i = 0; i < cs_n; ++i) {
        uint64_t con = cs[i];

        const ce_cdb_obj_o0 *conn_r = ce_cdb_a0->read(ce_cdb_a0->db(), con);

        uint64_t in_o = ce_cdb_a0->read_subobject(conn_r, CT_NODE_GRAPH_CONN_TO,
                                                  0);
        const ce_cdb_obj_o0 *in_r = ce_cdb_a0->read(ce_cdb_a0->db(), in_o);

        ct_node_i0 *ni = _get_node_i(in_r);

        if (!ni) {
            continue;
        }

        uint32_t in_n = 0;
        const ct_node_pin_def *in_def = ni->input_defs(&in_n);

        ce_vec2_t in_size = {
                ce_cdb_a0->read_float(in_r, CT_NODE_GRAPH_NODE_SIZE_X, 0),
                ce_cdb_a0->read_float(in_r, CT_NODE_GRAPH_NODE_SIZE_Y, 0)
        };

        ce_vec2_t in_pos = {
                ce_cdb_a0->read_float(in_r, CT_NODE_GRAPH_NODE_POS_X, 0),
                ce_cdb_a0->read_float(in_r, CT_NODE_GRAPH_NODE_POS_Y, 0)
        };

        uint64_t in_pin_name = ce_cdb_a0->read_uint64(conn_r,
                                                      CT_NODE_GRAPH_CONN_TO_PIN,
                                                      0);
        uint64_t in_pin_idx = _pin_idx_by_name(in_def, in_n, in_pin_name);


        ////


        uint64_t out_o = ce_cdb_a0->read_subobject(conn_r,
                                                   CT_NODE_GRAPH_CONN_FROM, 0);
        const ce_cdb_obj_o0 *out_r = ce_cdb_a0->read(ce_cdb_a0->db(), out_o);


        uint32_t out_n = 0;
        const ct_node_pin_def *out_def = ni->output_defs(&out_n);

        ce_vec2_t out_pos = {
                ce_cdb_a0->read_float(out_r, CT_NODE_GRAPH_NODE_POS_X, 0),
                ce_cdb_a0->read_float(out_r, CT_NODE_GRAPH_NODE_POS_Y, 0)
        };

        ce_vec2_t out_size = {
                ce_cdb_a0->read_float(out_r, CT_NODE_GRAPH_NODE_SIZE_X, 0),
                ce_cdb_a0->read_float(out_r, CT_NODE_GRAPH_NODE_SIZE_Y, 0)
        };

        uint64_t out_pin_name = ce_cdb_a0->read_uint64(conn_r,
                                                       CT_NODE_GRAPH_CONN_FROM_PIN,
                                                       0);

        uint64_t out_pin_idx = _pin_idx_by_name(out_def, out_n, out_pin_name);

        ce_vec2_t p1 = ce_vec2_add(offset,
                                   _get_node_out_pin_pos(out_pos, out_size,
                                                         out_n,
                                                         out_pin_idx));

        ce_vec2_t p2 = ce_vec2_add(offset,
                                   _get_node_in_pin_pos(in_pos, in_size,
                                                        in_n,
                                                        in_pin_idx));

        _draw_connection(&in_def[i], p1, p2, draw_list);
    }

}

static void draw_ng_editor(uint64_t graph,
                           uint64_t context) {
    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 60, 0, 200));
    ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    const bool isMouseDraggingForScrolling = ImGui::IsMouseDragging(2, 0.0f);
    const bool isMouseHoveringWindow = ImGui::IsWindowHovered(
            ImGuiHoveredFlags_AllowWhenBlockedByPopup |
            ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    const bool cantDragAnything = isMouseDraggingForScrolling;
    bool isLMBDraggingForMakingLinks =
            isMouseHoveringWindow && !cantDragAnything &&
            ImGui::IsMouseDragging(0, 0.0f);

    static ce_vec2_t scrolling = {};

    ce_vec2_t offset = ce_vec2_add(_to_vec2(ImGui::GetCursorScreenPos()),
                                   scrolling);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    _display_grid(scrolling, draw_list);


    draw_list->ChannelsSplit(2);

    _draw_connections(graph, offset, draw_list);
    _draw_nodes(graph, offset, draw_list, isLMBDraggingForMakingLinks, context);

    if (!isLMBDraggingForMakingLinks) {
        drag_node = {};
    }

    if (isLMBDraggingForMakingLinks && drag_node.from_obj) {
        _draw_dgraged_line(offset, draw_list);
    }

    draw_list->ChannelsMerge();

    // Scrolling
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDragging(1, 0.0f))
        scrolling = ce_vec2_add(scrolling, _to_vec2(ImGui::GetIO().MouseDelta));

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

static void draw_editor(uint64_t context_obj,
                        uint64_t context) {
//    draw_ng_editor(obj, context);
}

static uint64_t cdb_type() {
    return CT_NODE_GRAPH_ASSET;
}

static const char *display_icon() {
    return ICON_FA_CUBE;
}

static const char *display_name() {
    return "Node graph editor";
}


static struct ct_asset_editor_i0 ct_asset_editor_api = {
        .cdb_type = cdb_type,
        .open = open,
        .close = close,
        .update = update,
        .draw_ui = draw_editor,
        .display_name = display_name,
        .display_icon = display_icon,
};


static struct ct_node_graph_editor_a0 nge_api = {
        .add_node_modal= add_node_modal,
        .draw_ng_editor = draw_ng_editor,
};

struct ct_node_graph_editor_a0 *ct_node_graph_editor_a0 = &nge_api;


extern "C" {

void CE_MODULE_LOAD (node_graph_editor)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_api(CT_NODE_GRAPH_EDITOR_A0_STR,
                 ct_node_graph_editor_a0,
                 sizeof(nge_api));

    api->add_impl(CT_ASSET_EDITOR_I0_STR,
                  &ct_asset_editor_api,
                  sizeof(ct_asset_editor_i0));
}

void CE_MODULE_UNLOAD (node_graph_editor)(struct ce_api_a0 *api,
                                          int reload) {

    CE_UNUSED(api);
}
}