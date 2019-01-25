#include <math.h>

extern "C" {
#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/hashlib.h>
#include <celib/api_system.h>

#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/node_graph/node_graph.h>
#include <cetech/editor/resource_editor.h>

#include "../node_graph_editor.h"
};

#include <cetech/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/debugui/debugui.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/ecs/ecs.h>
#include <fnmatch.h>

#define _G node_graph_editor_globals
static struct _G {
    struct ce_alloc *allocator;
} _G;


static void close(uint64_t context_obj) {

}

static void open(uint64_t context_obj) {

}

static void update(uint64_t context_obj,
                   float dt) {

}


static char modal_buffer[128] = {};

static void add_node_modal(const char *modal_id,
                           uint64_t obj) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    bool open = true;
    ct_debugui_a0->SetNextWindowSize((float[2]) {512, 512},
                                     static_cast<DebugUICond>(0));
    if (ct_debugui_a0->BeginPopupModal(modal_id, &open, 0)) {
        struct ct_controlers_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

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


        struct ce_api_entry it = ce_api_a0->first(CT_NODE_I);
        while (it.api) {
            struct ct_node_i0 *i = static_cast<ct_node_i0 *>(it.api);
            struct ct_editor_node_i0 *ei;

            if (!i->get_interface) {
                it = ce_api_a0->next(it);
                continue;
            }

            ei = static_cast<ct_editor_node_i0 *>(i->get_interface(
                    CT_EDITOR_NODE_I));

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
                                                     (float[2]) {0.0f});

                if (add) {
                    uint64_t node = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                             CT_NODE_GRAPH_NODE);

                    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                             node);

                    ce_cdb_a0->set_uint64(w, CT_NODE_TYPE, i->cdb_type());

                    if (!ce_cdb_a0->prop_exist(w, CT_NODE_GRAPH_NODES)) {
                        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                               CT_NODE_GRAPH_NODES);
                        ce_cdb_a0->set_subobject(w, CT_NODE_GRAPH_NODES, ch);
                    }

                    if (!ce_cdb_a0->prop_exist(w, CT_NODE_GRAPH_CONNECTIONS)) {
                        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                               CT_NODE_GRAPH_CONNECTIONS);
                        ce_cdb_a0->set_subobject(w, CT_NODE_GRAPH_CONNECTIONS,
                                                 ch);
                    }

                    ce_cdb_a0->write_commit(w);

                    uint64_t nodes = ce_cdb_a0->read_subobject(reader,
                                                               CT_NODE_GRAPH_NODES,
                                                               0);
                    w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), nodes);
                    ce_cdb_a0->set_subobject(w, node, node);
                    ce_cdb_a0->write_commit(w);

                    modal_buffer[0] = '\0';
                }
            }

            it = ce_api_a0->next(it);
        }

        ct_debugui_a0->EndPopup();
    }
}


static inline ImVec2 operator+(const ImVec2 &lhs,
                               const ImVec2 &rhs) {
    return ImVec2(lhs.x + rhs.x,
                  lhs.y +
                  rhs.y);
}
//static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }


ImVec2 GetInputSlotPos(ImVec2 Pos,
                       ImVec2 Size,
                       uint32_t count,
                       int slot_no) {
    return ImVec2(Pos.x,
                  Pos.y + Size.y * ((float) slot_no + 1) / ((float) count + 1));
}

ImVec2 GetOutputSlotPos(ImVec2 Pos,
                        ImVec2 Size,
                        uint32_t count,
                        int slot_no) {
    return ImVec2(Pos.x + Size.x,
                  Pos.y + Size.y * ((float) slot_no + 1) / ((float) count + 1));
}

static void draw_ng_editor(uint64_t graph) {
    const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 60, 0, 200));
    ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    static ImVec2 scrolling = ImVec2(0.0f, 0.0f);

    ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    // Display grid
    if (true) {
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


    draw_list->ChannelsSplit(2);

    static int node_selected = -1;
    int node_hovered_in_list = -1;
    int node_hovered_in_scene = -1;

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), graph);
    uint64_t nodes = ce_cdb_a0->read_subobject(reader, CT_NODE_GRAPH_NODES, 0);
    const ce_cdb_obj_o *ns_reader = ce_cdb_a0->read(ce_cdb_a0->db(), nodes);
    const uint64_t *ns = ce_cdb_a0->prop_keys(ns_reader);
    const uint64_t ns_n = ce_cdb_a0->prop_count(ns_reader);
    for (int i = 0; i < ns_n; ++i) {
        uint64_t node = ns[i];

        ImGui::PushID(node);

        const ce_cdb_obj_o *nreader = ce_cdb_a0->read(ce_cdb_a0->db(), node);

        ImVec2 pos = {
                ce_cdb_a0->read_float(nreader,
                                      CT_NODE_GRAPH_NODE_POS_X, 0.0f),
                ce_cdb_a0->read_float(nreader,
                                      CT_NODE_GRAPH_NODE_POS_Y, 0.0f)
        };

        uint64_t node_type = ce_cdb_a0->read_uint64(nreader, CT_NODE_TYPE, 0);

        ct_node_i0 *ni = ct_node_graph_a0->get_interface(node_type);
        ct_editor_node_i0 *eni = (ct_editor_node_i0 *) ni->get_interface(
                CT_EDITOR_NODE_I);

        uint32_t input_n = 0;
        const ct_node_pin_def *inputs = ni->input_defs(&input_n);

        uint32_t output_n = 0;
        const ct_node_pin_def *outputs = ni->output_defs(&output_n);
        CE_UNUSED(outputs);

        const char *disply_name = eni->display_name();

        ImVec2 size;

        ImVec2 node_rect_min = offset + pos;

        // Display node contents first
        draw_list->ChannelsSetCurrent(1); // Foreground
        bool old_any_active = ImGui::IsAnyItemActive();
        ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
        ImGui::BeginGroup(); // Lock horizontal position
        ImGui::Text("%s", disply_name);

        uint32_t pin_n = input_n > output_n ? input_n : output_n;
        for (int k = 0; k < pin_n; ++k) {
            ImGui::Text("");
        }
        ImGui::EndGroup();

        bool node_widgets_active = (!old_any_active &&
                                    ImGui::IsAnyItemActive());
        size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING +
               NODE_WINDOW_PADDING;
        ImVec2 node_rect_max = node_rect_min + size;

        draw_list->ChannelsSetCurrent(0); // Background
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("node", size);
        if (ImGui::IsItemHovered()) {
            node_hovered_in_scene = node;
//            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        bool node_moving_active = ImGui::IsItemActive();
        if (node_widgets_active || node_moving_active)
            node_selected = node;

        if (node_moving_active && ImGui::IsMouseDragging(0)) {
            pos = pos + ImGui::GetIO().MouseDelta;

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), node);
            ce_cdb_a0->set_float(w, CT_NODE_GRAPH_NODE_POS_X, pos.x);
            ce_cdb_a0->set_float(w, CT_NODE_GRAPH_NODE_POS_Y, pos.y);
            ce_cdb_a0->write_commit(w);
        }


        const float NODE_SLOT_RADIUS = 4.0f;

        ImU32 node_bg_color = (node_hovered_in_list == node ||
                               node_hovered_in_scene == node ||
                               (node_hovered_in_list == -1 &&
                                node_selected == node)) ? IM_COL32(0, 75,
                                                                   0, 255)
                                                        : IM_COL32(0, 60,
                                                                   0, 255);
        draw_list->AddRectFilled(node_rect_min,
                                 node_rect_max, node_bg_color, 4.0f);

        draw_list->AddRect(node_rect_min, node_rect_max,
                           IM_COL32(0, 255, 0, 255), 4.0f);


        for (int j = 0; j < input_n; ++j) {
            const ct_node_pin_def *def = &inputs[j];
            ImVec2 pin_pos = offset + GetInputSlotPos(pos, size, input_n, j);
            const char *name = def->name;
            const char *name_end = name + strlen(name);
            draw_list->AddText(pin_pos, IM_COL32(0, 255, 0, 255), name,
                               name_end);
            draw_list->AddCircleFilled(pin_pos,
                                       NODE_SLOT_RADIUS,
                                       IM_COL32(0, 255, 0, 255));
        }

        for (int j = 0; j < output_n; ++j) {
            const ct_node_pin_def *def = &outputs[j];
            ImVec2 pin_pos = offset + GetOutputSlotPos(pos, size, output_n, j);

            const char *name = def->name;
            const char *name_end = name + strlen(name);
            draw_list->AddText(pin_pos, IM_COL32(0, 255, 0, 255), name,
                               name_end);

            draw_list->AddCircleFilled(pin_pos,
                                       NODE_SLOT_RADIUS,
                                       IM_COL32(0, 255, 0, 255));
        }


        ImGui::PopID();
    }
    draw_list->ChannelsMerge();

    // Scrolling
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDragging(1, 0.0f))
        scrolling = scrolling + ImGui::GetIO().MouseDelta;

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

static void draw_editor(uint64_t context_obj) {

    const ce_cdb_obj_o *creader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                  context_obj);


    uint64_t obj = ce_cdb_a0->read_ref(creader, RESOURCE_EDITOR_OBJ, 0);

    draw_ng_editor(obj);
}

static uint64_t cdb_type() {
    return CT_NODE_GRAPH_RESOURCE;
}

static const char *display_icon() {
    return ICON_FA_CUBE;
}

static const char *display_name() {
    return "Node graph editor";
}


static struct ct_resource_editor_i0 ct_resource_editor_i0 = {
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
void CE_MODULE_INITAPI(node_graph_editor)(struct ce_api_a0 *api) {
}

void CE_MODULE_LOAD (node_graph_editor)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api(CT_NODE_GRAPH_EDITOR_API, ct_node_graph_editor_a0);
    api->register_api(RESOURCE_EDITOR_I, &ct_resource_editor_i0);
}

void CE_MODULE_UNLOAD (node_graph_editor)(struct ce_api_a0 *api,
                                          int reload) {

    CE_UNUSED(api);
}
}