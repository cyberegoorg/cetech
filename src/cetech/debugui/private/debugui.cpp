#undef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include <celib/cdb.h>
#include <cetech/renderer/renderer.h>

#include <cetech/controlers/keyboard.h>
#include <celib/macros.h>
#include <celib/id.h>

#include <celib/fs.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>
#include <cetech/controlers/mouse.h>
#include <celib/log.h>

#include <cetech/controlers/controlers.h>
#include <cetech/kernel/kernel.h>

#include "celib/config.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/controlers/gamepad.h>
#include "imgui_wrap.inl"
#include <celib/os/vio.h>
#include <celib/os/window.h>
#include <celib/os/input.h>


static struct DebugUIGlobal {
    ce_alloc_t0 *allocator;
} _G;

static uint32_t lshift;
static uint32_t rshift;
static uint32_t lctrl;
static uint32_t rctrl;
static uint32_t lalt;
static uint32_t ralt;
static uint32_t lsuper;
static uint32_t rsuper;


static void begin() {
    uint8_t viewid = 255;

    ct_controlers_i0 *keyboard, *mouse, *gp;
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
    io.NavInputs[ImGuiNavInput_Cancel] = gp->button_state(0, gp->button_index( "b"));
    io.NavInputs[ImGuiNavInput_Input] = gp->button_state(0, gp->button_index("y"));
    io.NavInputs[ImGuiNavInput_Menu] = gp->button_state(0, gp->button_index("x"));
    io.NavInputs[ImGuiNavInput_DpadLeft] = gp->button_state(0,gp->button_index("dpad_left"));
    io.NavInputs[ImGuiNavInput_DpadRight] = gp->button_state(0, gp->button_index("dpad_right"));
    io.NavInputs[ImGuiNavInput_DpadUp] = gp->button_state(0,gp->button_index("dpad_up"));
    io.NavInputs[ImGuiNavInput_DpadDown] = gp->button_state(0, gp->button_index("dpad_down"));
    io.NavInputs[ImGuiNavInput_DpadDown] = gp->button_state(0, gp->button_index("dpad_down"));

    float left_axis[2] = {0};
    gp->axis(0, gp->axis_index("left"), left_axis);

    io.NavInputs[ImGuiNavInput_LStickUp] = left_axis[1] > 0 ? left_axis[1] : 0;
    io.NavInputs[ImGuiNavInput_LStickDown] = left_axis[1] < 0 ? -left_axis[1] : 0;
    io.NavInputs[ImGuiNavInput_LStickLeft] = left_axis[0] < 0 ? -left_axis[0] : 0;
    io.NavInputs[ImGuiNavInput_LStickRight] = left_axis[0] > 0 ? left_axis[0] : 0;

    io.NavInputs[ImGuiNavInput_FocusPrev] = gp->button_state(0,gp->button_index("right_shoulder"));
    io.NavInputs[ImGuiNavInput_FocusNext] = gp->button_state(0,gp->button_index("left_shoulder"));


    if (io.WantSetMousePos) {
        ce_window_t0 *win = ct_renderer_a0->get_main_window();
        win->warp_mouse(win->inst, (int) io.MousePos.x, (int) io.MousePos.y);
    }

    imguiBeginFrame(mp[0], h - mp[1], btn, wheel[1], w, h, 0, viewid);
}

static void render() {
    imguiEndFrame();
}


static void SaveDock(struct ce_vio_t0 *output) {
    char *buffer = NULL;
    ImGui::saveToYaml(&buffer, _G.allocator);

    output->vt->write(output->inst, buffer, 1, strlen(buffer));
}

static void LoadDock(const char *path) {
    ImGui::loadFromYaml(path, ce_ydb_a0, ce_ydb_a0, ce_cdb_a0);
}

static struct ct_debugui_a0 debugui_api = {
        .render = render,

        .Text = ImGui::Text,
        .TextV = ImGui::TextV,
        .TextColored = imgui_wrap::TextColored,
        .TextColoredV = imgui_wrap::TextColoredV,
        .TextDisabled = ImGui::TextDisabled,
        .TextDisabledV = ImGui::TextDisabledV,
        .TextWrapped = ImGui::TextWrapped,
        .TextWrappedV = ImGui::TextWrappedV,
        .TextUnformatted = ImGui::TextUnformatted,
        .LabelText = ImGui::LabelText,
        .LabelTextV = ImGui::LabelTextV,
        .Bullet = ImGui::Bullet,
        .BulletText = ImGui::BulletText,
        .BulletTextV = ImGui::BulletTextV,
        .Button = imgui_wrap::Button,
        .SmallButton = ImGui::SmallButton,
        .InvisibleButton = imgui_wrap::InvisibleButton,
        .Image = imgui_wrap::Image,
        .Image2 = imgui_wrap::Image2,
        .ImageButton = imgui_wrap::ImageButton,
        .Checkbox = ImGui::Checkbox,
        .CheckboxFlags = ImGui::CheckboxFlags,
        .RadioButton = ImGui::RadioButton,
        .RadioButton2 = imgui_wrap::RadioButton2,
        .Combo = ImGui::Combo,
        .Combo2 = imgui_wrap::Combo2,
        .Combo3 = imgui_wrap::Combo3,
        .ColorEdit3 = ImGui::ColorEdit3,
        .ColorEdit4 = ImGui::ColorEdit4,
        .PlotLines = imgui_wrap::PlotLines,
        .PlotLines2 = imgui_wrap::PlotLines2,
        .PlotHistogram = imgui_wrap::PlotHistogram,
        .PlotHistogram2 = imgui_wrap::PlotHistogram2,
        .ProgressBar = imgui_wrap::ProgressBar,
        .DragFloat = ImGui::DragFloat,
        .DragFloat2 = imgui_wrap::DragFloat2,
        .DragFloat3 = imgui_wrap::DragFloat3,
        .DragFloat4 = imgui_wrap::DragFloat4,
        .DragFloatRange2 = ImGui::DragFloatRange2,
        .DragInt = ImGui::DragInt,
        .DragInt2 = imgui_wrap::DragInt2,
        .DragInt3 = imgui_wrap::DragInt3,
        .DragInt4 = imgui_wrap::DragInt4,
        .DragIntRange2 = ImGui::DragIntRange2,
        .InputText = ImGui::InputText,
        .InputTextMultiline = imgui_wrap::InputTextMultiline,
        .InputFloat = ImGui::InputFloat,
        .InputFloat2 = imgui_wrap::InputFloat2,
        .InputFloat3 = imgui_wrap::InputFloat3,
        .InputFloat4 = imgui_wrap::InputFloat4,
        .InputInt = ImGui::InputInt,
        .InputInt2 = imgui_wrap::InputInt2,
        .InputInt3 = imgui_wrap::InputInt3,
        .InputInt4 = imgui_wrap::InputInt4,
        .SliderFloat = ImGui::SliderFloat,
        .SliderFloat2 = imgui_wrap::SliderFloat2,
        .SliderFloat3 = imgui_wrap::SliderFloat3,
        .SliderFloat4 = imgui_wrap::SliderFloat4,
        .SliderAngle = ImGui::SliderAngle,
        .SliderInt = ImGui::SliderInt,
        .SliderInt2 = ImGui::SliderInt2,
        .SliderInt3 = ImGui::SliderInt3,
        .SliderInt4 = ImGui::SliderInt4,
        .VSliderFloat = imgui_wrap::VSliderFloat,
        .VSliderInt = imgui_wrap::VSliderInt,
        .TreeNode = ImGui::TreeNode,
        .TreeNode2 = imgui_wrap::TreeNode2,
        .TreeNode3 = imgui_wrap::TreeNode3,
        .TreeNodeV = ImGui::TreeNodeV,
        .TreeNodeV2 = imgui_wrap::TreeNodeV2,
        .TreeNodeEx = ImGui::TreeNodeEx,
        .TreeNodeEx2 = imgui_wrap::TreeNodeEx2,
        .TreeNodeEx3 = imgui_wrap::TreeNodeEx3,
        .TreeNodeExV = imgui_wrap::TreeNodeExV,
        .TreeNodeExV2 = imgui_wrap::TreeNodeExV2,
        .TreePush = ImGui::TreePush,
        .TreePush2 = imgui_wrap::TreePush2,
        .TreePop = ImGui::TreePop,
        .TreeAdvanceToLabelPos = ImGui::TreeAdvanceToLabelPos,
        .GetTreeNodeToLabelSpacing = ImGui::GetTreeNodeToLabelSpacing,
        .SetNextTreeNodeOpen = ImGui::SetNextTreeNodeOpen,
        .CollapsingHeader = imgui_wrap::CollapsingHeader,
        .CollapsingHeader2 = imgui_wrap::CollapsingHeader2,
        .Selectable = imgui_wrap::Selectable,
        .Selectable2 = imgui_wrap::Selectable2,
        .ListBox = ImGui::ListBox,
        .ListBox2 = imgui_wrap::ListBox2,
        .ListBoxHeader = imgui_wrap::ListBoxHeader,
        .ListBoxHeader2 = imgui_wrap::ListBoxHeader2,
        .ListBoxFooter = imgui_wrap::ListBoxFooter,
        .Value = imgui_wrap::Value,
        .Value2 = imgui_wrap::Value2,
        .Value3 = imgui_wrap::Value3,
        .Value4 = imgui_wrap::Value4,

        .SetTooltip = ImGui::SetTooltip,
        .SetTooltipV = ImGui::SetTooltipV,
        .BeginTooltip = ImGui::BeginTooltip,
        .EndTooltip = ImGui::EndTooltip,
        .BeginMainMenuBar = ImGui::BeginMainMenuBar,
        .EndMainMenuBar = ImGui::EndMainMenuBar,
        .BeginMenuBar = ImGui::BeginMenuBar,
        .EndMenuBar = ImGui::EndMenuBar,
        .BeginMenu = ImGui::BeginMenu,
        .EndMenu = ImGui::EndMenu,
        .MenuItem = ImGui::MenuItem,
        .MenuItem2 = imgui_wrap::MenuItem2,
        .OpenPopup = ImGui::OpenPopup,
        .BeginPopup = ImGui::BeginPopup,
        .BeginPopupModal = ImGui::BeginPopupModal,
        .BeginPopupContextItem = ImGui::BeginPopupContextItem,
        .BeginPopupContextWindow = ImGui::BeginPopupContextWindow,
        .BeginPopupContextVoid = ImGui::BeginPopupContextVoid,
        .EndPopup = ImGui::EndPopup,
        .CloseCurrentPopup = ImGui::CloseCurrentPopup,
        .ColorWheel = ImGui::ColorWheel,
        .ColorWheel2 = imgui_wrap::ColorWheel2,
        .GetWindowSize = imgui_wrap::GetWindowSize,
        .GetWindowPos = imgui_wrap::GetWindowPos,
        .BeginDock = imgui_wrap::BeginDock,
        .EndDock = ImGui::EndDock,
        .IsWindowFocused = ImGui::IsWindowFocused,
        .IsMouseHoveringWindow = imgui_wrap::IsMouseHoveringWindow,
        .SameLine = ImGui::SameLine,
        .HSplitter = imgui_wrap::HSplitter,
        .VSplitter = imgui_wrap::VSplitter,
        .SaveDock = SaveDock,
        .LoadDock = LoadDock,
        .RootDock = imgui_wrap::RootDock,
        .IsMouseClicked = ImGui::IsMouseClicked,
        .IsMouseDoubleClicked = ImGui::IsMouseDoubleClicked,
        .IsItemClicked = ImGui::IsItemClicked,
        .IsItemHovered =ImGui::IsItemHovered,
        .Separator = ImGui::Separator,
        .GetItemRectMin = imgui_wrap::GetItemRectMin,
        .GetItemRectMax = imgui_wrap::GetItemRectMax,
        .GetItemRectSize = imgui_wrap::GetItemRectSize,
        .guizmo_set_rect = imgui_wrap::guizmo_set_rect,
        .guizmo_manipulate = imgui_wrap::guizmo_manipulate,
        .guizmo_decompose_matrix = imgui_wrap::guizmo_decompose_matrix,
        .GetContentRegionAvail= imgui_wrap::GetContentRegionAvail,
        .GetTextLineHeightWithSpacing = ImGui::GetTextLineHeightWithSpacing,


        .BeginDragDropSource = reinterpret_cast<bool (*)(DebugUIDragDropFlags_)>(ImGui::BeginDragDropSource),
        .SetDragDropPayload = reinterpret_cast<bool (*)(const char *,
                                                        const void *,
                                                        size_t,
                                                        DebugUICond)>(ImGui::SetDragDropPayload),
        .EndDragDropSource = ImGui::EndDragDropSource,
        .BeginDragDropTarget = ImGui::BeginDragDropTarget,
        .AcceptDragDropPayload = reinterpret_cast<const DebugUIPayload *(*)(const char *,
                                                                            DebugUIDragDropFlags_)>(ImGui::AcceptDragDropPayload),
        .EndDragDropTarget = ImGui::EndDragDropTarget,
        .BeginChild = imgui_wrap::BeginChild,

        .PushItemWidth = ImGui::PushItemWidth,
        .PopItemWidth= ImGui::PopItemWidth,

        .Columns = ImGui::Columns,
        .NextColumn = ImGui::NextColumn,
        .GetColumnIndex = ImGui::GetColumnIndex,
        .GetColumnWidth = ImGui::GetColumnWidth,
        .SetColumnWidth = ImGui::SetColumnWidth,
        .GetColumnOffset = ImGui::GetColumnOffset,
        .SetColumnOffset = ImGui::SetColumnOffset,
        .GetColumnsCount = ImGui::GetColumnsCount,
        .SetNextWindowSize = reinterpret_cast<void (*)(ce_vec2_t *,
                                                       DebugUICond)>(ImGui::SetNextWindowSize),

        .EndChild = ImGui::EndChild,
        .Unindent= ImGui::Unindent,
        .Indent= ImGui::Indent,
        .PushID = ImGui::PushID,
        .PushIDI=  ImGui::PushID,
        .PopID = ImGui::PopID,
        .PushItemFlag = ImGui::PushItemFlag,
        .PushStyleVar = ImGui::PushStyleVar,
        .PushColorStyleVar = imgui_wrap::PushColorStyleVar,
        .PopItemFlag = ImGui::PopItemFlag,
        .PopStyleVar = ImGui::PopStyleVar,
        .PopStyleColor = ImGui::PopStyleColor,
};

struct ct_debugui_a0 *ct_debugui_a0 = &debugui_api;


static uint64_t task_name() {
    return CT_DEBUGUI_TASK;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_INPUT_TASK,
            CT_RENDER_BEGIN_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static uint64_t *update_before(uint64_t *n) {
    static uint64_t a[] = {
            CT_RENDER_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static void debugui_update(float dt) {
    begin();
}

static struct ct_kernel_task_i0 debugui_task = {
        .name = task_name,
        .update = debugui_update,
        .update_after = update_after,
        .update_before= update_before,
};

static const char *_get_clipboard_text(void *data) {
    return ce_os_input_a0->get_clipboard_text();
}

static void _set_clipboard_text(void *data,
                                const char *text) {
    ce_os_input_a0->set_clipboard_text(text);
}

extern "C" {

void CE_MODULE_LOAD(debugui)(struct ce_api_a0 *api,
                             int reload) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_renderer_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_fs_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_UNUSED(reload);

    api->register_api(CT_DEBUGUI_API, &debugui_api, sizeof(debugui_api));
    api->register_api(KERNEL_TASK_INTERFACE, &debugui_task, sizeof(debugui_task));

    imguiCreate(12);

    _G = {
            .allocator = ce_memory_a0->system
    };

    ct_controlers_i0 *keyboard;
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

void CE_MODULE_UNLOAD(debugui)(struct ce_api_a0 *api,
                               int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    imguiDestroy();

    _G = {};
}

}