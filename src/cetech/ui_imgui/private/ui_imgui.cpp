#include <celib/cdb.h>

#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/macros.h>
#include <celib/id.h>
#include <celib/api.h>
#include <celib/module.h>
#include <celib/os/vio.h>
#include <celib/os/window.h>

#include <cetech/ui/ui.h>
#include <cetech/renderer/gfx.h>
#include <cetech/ui_imgui/private/ocornut-imgui/imgui.h>
#include <cetech/ui_imgui/private/ocornut-imgui/imgui_internal.h>
#include <celib/os/input.h>
#include <cetech/kernel/kernel.h>
#include <cetech/ui_imgui/private/bgfx_imgui/imgui.h>
#include <cetech/renderer/renderer.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/controlers/mouse.h>
#include <cetech/controlers/gamepad.h>

#define _G UIImguiGlobal
static struct _G {
    uint64_t last_id;
    ce_alloc_t0 *allocator;
} _G;

static inline ce_vec2_t _vec2_from(ImVec2 v) {
    return {v.x, v.y};
}


static inline ImVec2 _vec2_to_imguivec2(ce_vec2_t v) {
    return {v.x, v.y};
}

static inline ImVec4 _vec4_to_imguivec4(ce_vec4_t v) {
    return {v.x, v.y, v.z, v.w};
}

uint64_t generate_id() {
    return ++_G.last_id;
}

static void text(const char *text) {
    ImGui::Text(text);
}

void text_unformated(const char *text,
                     const char *text_end) {
    ImGui::TextUnformatted(text, text_end);
}

static bool button(const ct_ui_button_t0 *button) {
    return ImGui::Button(button->text, _vec2_to_imguivec2(button->size));
}

static void same_line(float pos_x,
                      float spacing_w) {
    ImGui::SameLine(pos_x, spacing_w);
}

static void next_column() {
    ImGui::NextColumn();
}

bool menu_begin(const ct_ui_menu_t0 *menu) {
    return ImGui::BeginMenu(menu->text, !menu->disabled);
}

static void menu_end() {
    ImGui::EndMenu();
}

bool menu_item(const ct_ui_menu_item_t0 *item) {
    return ImGui::MenuItem(item->text, item->shortcut, item->selected, !item->disabled);
}

bool menu_item_checkbox(const ct_ui_menu_item_t0 *item,
                        bool *selected) {
    return ImGui::MenuItem(item->text, item->shortcut, selected, !item->disabled);
}


bool begin_main_menu_bar() {
    return ImGui::BeginMainMenuBar();
}

void end_main_menu_bar() {
    ImGui::EndMainMenuBar();
}

bool begin_menu_bar() {
    return ImGui::BeginMenuBar();
}

void end_menu_bar() {
    ImGui::EndMenuBar();
}

static bool popup_begin(const ct_ui_popup_t0 *popup) {
    char modal_name[128];
    snprintf(modal_name, CE_ARRAY_LEN(modal_name), "###%llx", popup->id);

    return ImGui::BeginPopup(modal_name);
}

static void popup_end() {
    ImGui::EndPopup();
}

static void popup_close_current() {
    ImGui::CloseCurrentPopup();
}

static void popup_open(uint64_t id) {
    char modal_name[128];
    snprintf(modal_name, CE_ARRAY_LEN(modal_name), "###%llx", id);

    ImGui::OpenPopup(modal_name);
}

static bool modal_popup_begin(const ct_ui_modal_popup_t0 *modal,
                              bool *p_open) {
    char modal_name[128];
    snprintf(modal_name, CE_ARRAY_LEN(modal_name), "%s###%llx", modal->text, modal->id);
    return ImGui::BeginPopupModal(modal_name, p_open);
}

static void modal_popup_end() {
    ImGui::EndPopup();
}

static void modal_popup_open(uint64_t modal_id) {
    char modal_name[128];
    snprintf(modal_name, CE_ARRAY_LEN(modal_name), "###%llx", modal_id);
    ImGui::OpenPopup(modal_name);
}

void set_next_window_size(ce_vec2_t size) {
    ImGui::SetNextWindowSize(_vec2_to_imguivec2(size));
}

ce_vec2_t get_window_size() {
    return _vec2_from(ImGui::GetWindowSize());
}

void columns(uint64_t id,
             uint32_t count,
             bool border) {
    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "###%llx", id);

    ImGui::Columns(count, label, border);
}

bool input_text(const ct_ui_input_text_t0 *input,
                char *buf,
                size_t buf_size) {

    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "###%llx", input->id);
    bool ret = ImGui::InputText(label, buf, buf_size);

    return ret;
}

bool combo(const ct_ui_combo_t0 *combo,
           int32_t *current_item) {
    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "###%llx", combo->id);

    return ImGui::Combo(label, (int *) current_item, combo->items, combo->items_count);
}

bool combo2(const ct_ui_combo2_t0 *combo,
            int32_t *current_item,
            const char *items_separated_by_zeros) {
    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "###%llx", combo->id);

    return ImGui::Combo(label, (int *) current_item, items_separated_by_zeros);
}

bool child_begin(const ct_ui_child_t0 *child) {
    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "###%llx", child->id);
    bool ret = ImGui::BeginChild(label, _vec2_to_imguivec2(child->size), child->border);

    return ret;
}

void child_end() {
    ImGui::EndChild();
}

void push_item_width(float item_width) {
    ImGui::PushItemWidth(item_width);
}

void pop_item_width() {
    ImGui::PopItemWidth();
}

void tooltip_begin() {
    ImGui::BeginTooltip();
}

void tooltip_end() {
    ImGui::EndTooltip();
}

bool selectable(const ct_ui_selectable_t0 *selectable) {
    ImGuiSelectableFlags flags = 0;
    flags |= selectable->dont_close_popups ? ImGuiSelectableFlags_DontClosePopups : 0;
    flags |= selectable->allow_double_click ? ImGuiSelectableFlags_AllowDoubleClick : 0;

    return ImGui::Selectable(selectable->text,
                             selectable->selected,
                             flags,
                             _vec2_to_imguivec2(selectable->size));
}


bool is_item_hovered(ct_ui_hovered_flag_e0 flags) {
    return ImGui::IsItemHovered(flags);
}


bool is_mouse_double_clicked(uint32_t btn) {
    return ImGui::IsMouseDoubleClicked(btn);
}

bool is_mouse_clicked(uint32_t btn,
                      bool repeat) {
    return ImGui::IsMouseClicked(btn, repeat);
}

bool is_mouse_hovering_window() {
    return ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup |
                                  ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
}

bool is_item_clicked(uint32_t btn) {
    return ImGui::IsItemClicked(btn);
}

bool drag_drop_source_begin(enum ct_ui_drag_drop_0 flags) {
    return ImGui::BeginDragDropSource(flags);
}

void drag_drop_source_end() {
    return ImGui::EndDragDropSource();
}

bool set_drag_drop_payload(ct_ui_drop_payload_t0 *payload) {
    return ImGui::SetDragDropPayload(payload->type, payload->data, payload->size, payload->cond);
}


bool drag_drop_target_begin() {
    return ImGui::BeginDragDropTarget();
}

void drag_drop_target_end() {
    ImGui::EndDragDropTarget();
}

const void *accept_drag_drop_payload(const char *type) {
    const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type);
    if (!payload) {
        return NULL;
    }

    return payload->Data;
}

void image(const ct_ui_image_t0 *image) {
    bool bl = ct_gfx_a0->bgfx_get_caps()->originBottomLeft;
    ce_vec2_t uv0 = bl ? (ce_vec2_t) {0.0f, 1.0f} : (ce_vec2_t) {0.0f, 0.0f};
    ce_vec2_t uv1 = bl ? (ce_vec2_t) {1.0f, 0.0f} : (ce_vec2_t) {1.0f, 1.0f};


    ImGui::Image(reinterpret_cast<ImTextureID>(image->user_texture_id),
                 _vec2_to_imguivec2(image->size),
                 _vec2_to_imguivec2(uv0),
                 _vec2_to_imguivec2(uv1),
                 _vec4_to_imguivec4(image->tint_col),
                 _vec4_to_imguivec4(image->border_col));
}

ce_vec2_t get_content_region_avail() {
    return _vec2_from(ImGui::GetContentRegionAvail());
}

bool checkbox(const ct_ui_checkbox_t0 *checkbox,
              bool *v) {
    return ImGui::Checkbox(checkbox->text, v);
}

void dock_root(ce_vec2_t pos,
               ce_vec2_t size) {
    ImGui::RootDock(_vec2_to_imguivec2(pos), _vec2_to_imguivec2(size));
}

bool dock_begin(const ct_ui_dock_t0 *dock,
                bool *opened) {
    return ImGui::BeginDock(dock->label, opened, dock->flags);
}

void dock_end() {
    ImGui::EndDock();
}

void dock_save(struct ce_vio_t0 *output) {
    char *buffer = NULL;
    ImGui::saveToYaml(&buffer, _G.allocator);

    output->vt->write(output->inst, buffer, 1, strlen(buffer));
}

void dock_load(const char *path) {
    ImGui::loadFromYaml(path, ce_yaml_cdb_a0, ce_yaml_cdb_a0, ce_cdb_a0);
}

void separator() {
    ImGui::Separator();
}

void tree_pop() {
    ImGui::TreePop();
}

void push_item_flag(enum ct_ui_item_flags_e0 flags,
                    bool enabled) {
    ImGui::PushItemFlag(flags, enabled);
}

void push_style_var(enum ct_ui_style_var_e0 var,
                    float val) {
    ImGui::PushStyleVar(var, val);
}

void pop_item_flag() {
    ImGui::PopItemFlag();
}

void pop_style_var(uint32_t count) {
    ImGui::PopStyleVar(count);
}

void pop_style_color(uint32_t count) {
    ImGui::PopStyleColor(count);
}

bool drag_float(const ct_ui_drag_float_t0 *drag_float,
                float *v) {
    char id[128];
    snprintf(id, CE_ARRAY_LEN(id), "###%llx", drag_float->id);

    return ImGui::DragFloat(id, v,
                            drag_float->v_speed ? drag_float->v_speed : 1.0f,
                            drag_float->v_min,
                            drag_float->v_max,
                            drag_float->display_format ? drag_float->display_format : "%.3f",
                            drag_float->power ? drag_float->power : 1.0f
    );
}

bool drag_float2(const ct_ui_drag_float_t0 *drag_float,
                 ce_vec2_t *v) {
    char id[128];
    snprintf(id, CE_ARRAY_LEN(id), "###%llx", drag_float->id);

    return ImGui::DragFloat2(id, &v->x,
                             drag_float->v_speed ? drag_float->v_speed : 1.0f,
                             drag_float->v_min,
                             drag_float->v_max,
                             drag_float->display_format ? drag_float->display_format : "%.3f",
                             drag_float->power ? drag_float->power : 1.0f
    );
}

bool drag_float3(const ct_ui_drag_float_t0 *drag_float,
                 ce_vec3_t *v) {
    char id[128];
    snprintf(id, CE_ARRAY_LEN(id), "###%llx", drag_float->id);

    return ImGui::DragFloat3(id, &v->x,
                             drag_float->v_speed ? drag_float->v_speed : 1.0f,
                             drag_float->v_min,
                             drag_float->v_max,
                             drag_float->display_format ? drag_float->display_format : "%.3f",
                             drag_float->power ? drag_float->power : 1.0f
    );
}

bool drag_float4(const ct_ui_drag_float_t0 *drag_float,
                 ce_vec4_t *v) {
    char id[128];
    snprintf(id, CE_ARRAY_LEN(id), "###%llx", drag_float->id);

    return ImGui::DragFloat4(id, &v->x,
                             drag_float->v_speed ? drag_float->v_speed : 1.0f,
                             drag_float->v_min,
                             drag_float->v_max,
                             drag_float->display_format ? drag_float->display_format : "%.3f",
                             drag_float->power ? drag_float->power : 1.0f
    );
}

bool drag_int(const ct_ui_drag_int_t0 *drag_int,
              int32_t *v) {
    char id[128];
    snprintf(id, CE_ARRAY_LEN(id), "###%llx", drag_int->id);

    return ImGui::DragInt(id, v,
                          drag_int->v_speed ? drag_int->v_speed : 1.0f,
                          drag_int->v_min,
                          drag_int->v_max,
                          drag_int->display_format ? drag_int->display_format : "%.3f"
    );
}

bool color_edit(const ct_ui_color_edit_t0 *color,
                ce_vec4_t *c) {
    char id[128];
    snprintf(id, CE_ARRAY_LEN(id), "###%llx", color->id);

    return ImGui::ColorEdit4(id, &c->x);
}

bool tree_node_ex(const ct_ui_tree_node_ex_t0 *node) {
    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "%s##%llx", node->text, node->id);

    return ImGui::TreeNodeEx(label, node->flags);
}

bool collapsing_header(const ct_ui_collapsing_header_t0 *header) {
    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "%s##%llx", header->text, header->id);

    return ImGui::CollapsingHeader(label, header->flags);
}


bool radio_button2(const ct_ui_radio_buttton_t0 *btn,
                   int32_t *v) {
    char label[128];
    snprintf(label, CE_ARRAY_LEN(label), "%s##%llx", btn->text, btn->id);
    return ImGui::RadioButton(label, v, btn->value);
}

void plot_lines(const ct_ui_plot_lines_t0 *plot) {
    char id[128];
    snprintf(id, CE_ARRAY_LEN(id), "###%llx", plot->id);

    ImGui::PlotLines(id,
                     plot->values,
                     plot->values_count,
                     plot->values_offset,
                     plot->overlay_text,
                     plot->scale_min,
                     plot->scale_max,
                     _vec2_to_imguivec2(plot->graph_size));
}

void push_id(uint64_t id) {
    ImGui::PushID((void *) id);
}

void pop_id() {
    ImGui::PopID();
}

static uint32_t lshift;
static uint32_t rshift;
static uint32_t lctrl;
static uint32_t rctrl;
static uint32_t lalt;
static uint32_t ralt;
static uint32_t lsuper;
static uint32_t rsuper;


static void begin(float dt) {
    uint8_t viewid = 255;

    ct_controler_i0 *keyboard, *mouse, *gp;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);
    mouse = ct_controlers_a0->get(CONTROLER_MOUSE);
    gp = ct_controlers_a0->get(CONTROLER_GAMEPAD);

    float mp[3] = {};
    mouse->axis(0, mouse->axis_index("absolute"), mp);

    uint8_t btn = 0;

    if (mouse->button_state(0, mouse->button_index("left")) != 0) {
        btn |= IMGUI_MBUT_LEFT;
    }

    if (mouse->button_state(0, mouse->button_index("right")) != 0) {
        btn |= IMGUI_MBUT_RIGHT;
    }

    if (mouse->button_state(0, mouse->button_index("midle")) != 0) {
        btn |= IMGUI_MBUT_MIDDLE;
    }

    uint32_t w, h;
    ct_renderer_a0->get_size(&w, &h);

    const uint32_t axis = mouse->axis_index("wheel");
    float wheel[2];
    mouse->axis(0, axis, wheel);

    ImGuiIO &io = ImGui::GetIO();

    for (uint32_t i = 0; i < 512; ++i) {
        io.KeysDown[i] = keyboard->button_state(0, i) > 0;
    }

    if (!io.KeysDown[40]) {
        io.KeysDown[40] = keyboard->button_state(0, 88) > 0;
    }

    io.KeyShift = (keyboard->button_state(0, lshift) > 0)
                  || (keyboard->button_state(0, rshift) > 0);

    io.KeyCtrl = (keyboard->button_state(0, lctrl) > 0)
                 || (keyboard->button_state(0, rctrl) > 0);

    io.KeyAlt = (keyboard->button_state(0, lalt) > 0)
                || (keyboard->button_state(0, ralt) > 0);

    io.KeySuper = (keyboard->button_state(0, lsuper) > 0)
                  || (keyboard->button_state(0, rsuper) > 0);

    char *txt = keyboard->text(0);
    if (txt[0]) {
        io.AddInputCharactersUTF8(txt);
    }

    io.NavInputs[ImGuiNavInput_Activate] = gp->button_state(0, gp->button_index("a"));
    io.NavInputs[ImGuiNavInput_Cancel] = gp->button_state(0, gp->button_index("b"));
    io.NavInputs[ImGuiNavInput_Input] = gp->button_state(0, gp->button_index("y"));
    io.NavInputs[ImGuiNavInput_Menu] = gp->button_state(0, gp->button_index("x"));
    io.NavInputs[ImGuiNavInput_DpadLeft] = gp->button_state(0, gp->button_index("dpad_left"));
    io.NavInputs[ImGuiNavInput_DpadRight] = gp->button_state(0, gp->button_index("dpad_right"));
    io.NavInputs[ImGuiNavInput_DpadUp] = gp->button_state(0, gp->button_index("dpad_up"));
    io.NavInputs[ImGuiNavInput_DpadDown] = gp->button_state(0, gp->button_index("dpad_down"));
    io.NavInputs[ImGuiNavInput_DpadDown] = gp->button_state(0, gp->button_index("dpad_down"));

    float left_axis[2] = {0};
    gp->axis(0, gp->axis_index("left"), left_axis);

    io.NavInputs[ImGuiNavInput_LStickUp] = left_axis[1] > 0 ? left_axis[1] : 0;
    io.NavInputs[ImGuiNavInput_LStickDown] = left_axis[1] < 0 ? -left_axis[1] : 0;
    io.NavInputs[ImGuiNavInput_LStickLeft] = left_axis[0] < 0 ? -left_axis[0] : 0;
    io.NavInputs[ImGuiNavInput_LStickRight] = left_axis[0] > 0 ? left_axis[0] : 0;

    io.NavInputs[ImGuiNavInput_FocusPrev] = gp->button_state(0, gp->button_index("right_shoulder"));
    io.NavInputs[ImGuiNavInput_FocusNext] = gp->button_state(0, gp->button_index("left_shoulder"));


    if (io.WantSetMousePos) {
        ce_window_t0 *win = ct_renderer_a0->get_main_window();
        win->warp_mouse(win->inst, (int) io.MousePos.x, (int) io.MousePos.y);
    }

    imguiBeginFrame(mp[0], h - mp[1], btn, wheel[1], w, h, 0, viewid);
}

static struct ct_kernel_task_i0 debugui_task = {
        .name = CT_UI_TASK,
        .update = begin,
        .update_after = CT_KERNEL_AFTER(CT_INPUT_TASK,
                                        CT_RENDER_BEGIN_TASK),
        .update_before= CT_KERNEL_BEFORE(CT_RENDER_TASK),
};

static const char *_get_clipboard_text(void *data) {
    return ce_os_input_a0->get_clipboard_text();
}

static void _set_clipboard_text(void *data,
                                const char *text) {
    ce_os_input_a0->set_clipboard_text(text);
}


static struct ct_ui_a0 ui_api = {
        .render = imguiEndFrame,
        .generate_id = generate_id,
        .text=text,
        .text_unformated = text_unformated,
        .button = button,
        .input_text = input_text,
        .combo = combo,
        .combo2 = combo2,
        .drag_float = drag_float,
        .drag_float2 = drag_float2,
        .drag_float3 = drag_float3,
        .drag_float4 = drag_float4,
        .drag_int = drag_int,
        .color_edit = color_edit,
        .radio_button2 = radio_button2,
        .plot_lines = plot_lines,

        .selectable = selectable,
        .checkbox = checkbox,

        .child_begin = child_begin,
        .child_end = child_end,
        .push_item_width = push_item_width,
        .pop_item_width = pop_item_width,
        .same_line = same_line,
        .next_column = next_column,

        .menu_begin = menu_begin,
        .menu_end = menu_end,
        .menu_item = menu_item,
        .menu_item_checkbox = menu_item_checkbox,
        .begin_main_menu_bar = begin_main_menu_bar,
        .end_main_menu_bar = end_main_menu_bar,
        .begin_menu_bar = begin_menu_bar,
        .end_menu_bar = end_menu_bar,

        .popup_open = popup_open,
        .popup_begin = popup_begin,
        .popup_end = popup_end,
        .popup_close_current = popup_close_current,

        .modal_popup_begin = modal_popup_begin,
        .modal_popup_end = modal_popup_end,
        .modal_popup_open = modal_popup_open,

        .set_next_window_size = set_next_window_size,
        .get_window_size = get_window_size,

        .columns = columns,

        .tooltip_begin = tooltip_begin,
        .tooltip_end = tooltip_end,

        .is_item_hovered = is_item_hovered,
        .is_mouse_double_clicked = is_mouse_double_clicked,
        .is_mouse_clicked = is_mouse_clicked,
        .is_mouse_hovering_window = is_mouse_hovering_window,
        .is_item_clicked = is_item_clicked,

        .drag_drop_source_begin = drag_drop_source_begin,
        .drag_drop_source_end = drag_drop_source_end,
        .set_drag_drop_payload = set_drag_drop_payload,
        .drag_drop_target_begin = drag_drop_target_begin,
        .drag_drop_target_end = drag_drop_target_end,
        .accept_drag_drop_payload = accept_drag_drop_payload,

        .get_content_region_avail = get_content_region_avail,

        .dock_root = dock_root,
        .dock_begin = dock_begin,
        .dock_end = dock_end,
        .image = image,
        .dock_save= dock_save,
        .dock_load = dock_load,
        .separator = separator,
        .tree_pop = tree_pop,
        .push_item_flag = push_item_flag,
        .push_style_var = push_style_var,
        .pop_item_flag = pop_item_flag,
        .pop_style_var = pop_style_var,
        .pop_style_color = pop_style_color,

        .tree_node_ex = tree_node_ex,
        .collapsing_header = collapsing_header,

        .push_id = push_id,
        .pop_id = pop_id,
};

struct ct_ui_a0 *ct_ui_a0 = &ui_api;

extern "C" {
void CE_MODULE_LOAD(ui_imgui)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_api(CT_UI_A0_STR, &ui_api, sizeof(ui_api));

    api->add_impl(CT_KERNEL_TASK_I0_STR, &debugui_task, sizeof(debugui_task));

    imguiCreate(18);

    ct_controler_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);

    lshift = keyboard->button_index("lshift");
    rshift = keyboard->button_index("rshift");
    lctrl = keyboard->button_index("lctrl");
    rctrl = keyboard->button_index("rctrl");
    lalt = keyboard->button_index("lalt");
    ralt = keyboard->button_index("ralt");
    lsuper = keyboard->button_index("lsuper");
    rsuper = keyboard->button_index("rsuper");

    ImGuiIO &io = ImGui::GetIO();

    io.SetClipboardTextFn = _set_clipboard_text;
    io.GetClipboardTextFn = _get_clipboard_text;
    io.ClipboardUserData = NULL;

    io.KeyMap[ImGuiKey_Tab] = keyboard->button_index("tab");
    io.KeyMap[ImGuiKey_LeftArrow] = keyboard->button_index("left");
    io.KeyMap[ImGuiKey_RightArrow] = keyboard->button_index("right");
    io.KeyMap[ImGuiKey_UpArrow] = keyboard->button_index("up");
    io.KeyMap[ImGuiKey_DownArrow] = keyboard->button_index("down");
    io.KeyMap[ImGuiKey_PageUp] = keyboard->button_index("pageup");
    io.KeyMap[ImGuiKey_PageDown] = keyboard->button_index("pagedown");
    io.KeyMap[ImGuiKey_Home] = keyboard->button_index("home");
    io.KeyMap[ImGuiKey_End] = keyboard->button_index("end");
    io.KeyMap[ImGuiKey_Insert] = keyboard->button_index("insert");
    io.KeyMap[ImGuiKey_Delete] = keyboard->button_index("delete");
    io.KeyMap[ImGuiKey_Backspace] = keyboard->button_index("backspace");
    io.KeyMap[ImGuiKey_Enter] = keyboard->button_index("enter");
    io.KeyMap[ImGuiKey_Escape] = keyboard->button_index("escape");
    io.KeyMap[ImGuiKey_Space] = keyboard->button_index("space");

    io.KeyMap[ImGuiKey_A] = keyboard->button_index("a");
    io.KeyMap[ImGuiKey_C] = keyboard->button_index("c");
    io.KeyMap[ImGuiKey_V] = keyboard->button_index("v");
    io.KeyMap[ImGuiKey_X] = keyboard->button_index("x");
    io.KeyMap[ImGuiKey_Y] = keyboard->button_index("y");
    io.KeyMap[ImGuiKey_Z] = keyboard->button_index("z");


    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    ImVec4 *colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.36f, 0.00f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.15f, 0.00f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.20f, 0.00f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.27f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.55f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.78f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.20f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.36f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.20f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.20f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.20f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.65f, 0.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.84f, 0.00f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 0.89f, 0.00f, 0.90f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.78f, 0.00f, 0.90f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.73f, 0.02f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.00f, 1.00f, 0.00f, 0.90f);

    colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.26f, 0.59f, 0.00f, 0.40f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.26f, 0.59f, 0.00f, 0.40f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.00f, 0.40f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.00f, 0.40f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 1.02f, 0.00f, 0.40f);
}

void CE_MODULE_UNLOAD(ui_imgui)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    imguiDestroy();
}
}