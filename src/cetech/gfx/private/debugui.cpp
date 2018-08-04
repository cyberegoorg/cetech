#undef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include <celib/cdb.h>
#include <cetech/gfx/renderer.h>
#include <cetech/gfx/debugui.h>
#include <cetech/controlers/keyboard.h>
#include <celib/hashlib.h>
#include <celib/os.h>
#include <celib/fs.h>
#include <celib/array.inl>
#include <cetech/controlers/mouse.h>
#include <celib/log.h>
#include <celib/ebus.h>
#include <cetech/controlers/controlers.h>

#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"

#include "imgui_wrap.inl"


static struct DebugUIGlobal {
    ce_alloc *allocator;
} _G;

static void render(uint8_t viewid) {
    struct ct_controlers_i0 *keyboard, *mouse;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);
    mouse = ct_controlers_a0->get(CONTROLER_MOUSE);

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

    io.KeyShift = (keyboard->button_state(0, keyboard->button_index(
            "lshift")) > 0) || (keyboard->button_state(0,
                                                       keyboard->button_index(
                                                               "rshift")) >
                                0);
    io.KeyCtrl = (keyboard->button_state(0, keyboard->button_index(
            "lctrl")) > 0) || (keyboard->button_state(0,
                                                      keyboard->button_index(
                                                              "rctrl")) >
                               0);
    io.KeyAlt = (keyboard->button_state(0, keyboard->button_index(
            "lalt")) > 0) || (keyboard->button_state(0,
                                                     keyboard->button_index(
                                                             "ralt")) >
                              0);
    io.KeySuper = (keyboard->button_state(0, keyboard->button_index(
            "super")) > 0);

    char *txt = keyboard->text(0);
    if (txt[0]) {
        io.AddInputCharactersUTF8(txt);
    }

    imguiBeginFrame(mp[0], h - mp[1], btn, wheel[1], w, h, 0, viewid);

    uint64_t event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                              DEBUGUI_EVENT);

    ce_ebus_a0->broadcast(DEBUGUI_EBUS, event);
    imguiEndFrame();
}

static void SaveDock(struct ce_vio *output) {
    char *buffer = NULL;
    ImGui::saveToYaml(&buffer, _G.allocator);

    output->write(output, buffer, 1, strlen(buffer));
}

static void LoadDock(const char *path) {
    ImGui::loadFromYaml(path, ce_ydb_a0, ce_yng_a0);
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
};

struct ct_debugui_a0 *ct_debugui_a0 = &debugui_api;

static void _init(struct ce_api_a0 *api) {
    api->register_api("ct_debugui_a0", &debugui_api);
    imguiCreate(12);

    _G = {
            .allocator = ce_memory_a0->system
    };

    ce_ebus_a0->create_ebus("debugui", DEBUGUI_EBUS);


    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);


    ImGuiIO &io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = keyboard->button_index("tab");
    io.KeyMap[ImGuiKey_LeftArrow] = keyboard->button_index("left");
    io.KeyMap[ImGuiKey_RightArrow] = keyboard->button_index("right");
    io.KeyMap[ImGuiKey_UpArrow] = keyboard->button_index("up");
    io.KeyMap[ImGuiKey_DownArrow] = keyboard->button_index("down");
    io.KeyMap[ImGuiKey_PageUp] = keyboard->button_index("pageup");
    io.KeyMap[ImGuiKey_PageDown] = keyboard->button_index("pagedown");
    io.KeyMap[ImGuiKey_Home] = keyboard->button_index("home");
    io.KeyMap[ImGuiKey_End] = keyboard->button_index("end");
    io.KeyMap[ImGuiKey_Delete] = keyboard->button_index("delete");
    io.KeyMap[ImGuiKey_Backspace] = keyboard->button_index("backspace");
    io.KeyMap[ImGuiKey_Enter] = keyboard->button_index("return");
    io.KeyMap[ImGuiKey_Escape] = keyboard->button_index("escape");

    io.KeyMap[ImGuiKey_A] = keyboard->button_index("a");
    io.KeyMap[ImGuiKey_C] = keyboard->button_index("c");
    io.KeyMap[ImGuiKey_V] = keyboard->button_index("v");
    io.KeyMap[ImGuiKey_X] = keyboard->button_index("x");
    io.KeyMap[ImGuiKey_Y] = keyboard->button_index("y");
    io.KeyMap[ImGuiKey_Z] = keyboard->button_index("z");
}

static void _shutdown() {
    imguiDestroy();

    _G = {};
}

CE_MODULE_DEF(
        debugui,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_renderer_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_fs_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_yng_a0);
            CE_INIT_API(api, ce_log_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ce_cdb_a0);
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