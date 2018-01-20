#include <cetech/engine/entity/entity.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/input/input.h>
#include <cetech/engine/application/application.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/engine/renderer/viewport.h>
#include <cetech/core/os/vio.h>
#include <cetech/engine/filesystem/filesystem.h>
#include <cetech/core/containers/array.h>
#include "cetech/core/containers/map.inl"

#include "cetech/engine/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

#include "imgui_wrap.inl"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_mouse_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);

using namespace celib;

typedef void (on_debug_t)();

static struct DebugUIGlobal {
    on_debug_t **on_debugui;
    ct_alloc *allocator;
} _G;

static void render(uint8_t viewid) {
    float mp[3] = {};
    ct_mouse_a0.axis(0, ct_mouse_a0.axis_index("absolute"), mp);

    uint8_t btn = 0;

    if (ct_mouse_a0.button_state(0, ct_mouse_a0.button_index("left")) !=
        0) {
        btn |= IMGUI_MBUT_LEFT;
    }

    if (ct_mouse_a0.button_state(0, ct_mouse_a0.button_index("right")) !=
        0) {
        btn |= IMGUI_MBUT_RIGHT;
    }

    if (ct_mouse_a0.button_state(0, ct_mouse_a0.button_index("midle")) !=
        0) {
        btn |= IMGUI_MBUT_MIDDLE;
    }

    uint32_t w, h;
    ct_renderer_a0.get_size(&w, &h);

    const uint32_t axis = ct_mouse_a0.axis_index("wheel");
    float wheel[2];
    ct_mouse_a0.axis(0, axis, wheel);

    ImGuiIO &io = ImGui::GetIO();

    for (uint32_t i = 0; i < 512; ++i) {
        io.KeysDown[i] = ct_keyboard_a0.button_state(0, i) > 0;
    }

    if (!io.KeysDown[40]) {
        io.KeysDown[40] = ct_keyboard_a0.button_state(0, 88) > 0;
    }

    io.KeyShift = (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index(
            "lshift")) > 0) || (ct_keyboard_a0.button_state(0,
                                                            ct_keyboard_a0.button_index(
                                                                    "rshift")) >
                                0);
    io.KeyCtrl = (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index(
            "lctrl")) > 0) || (ct_keyboard_a0.button_state(0,
                                                           ct_keyboard_a0.button_index(
                                                                   "rctrl")) >
                               0);
    io.KeyAlt = (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index(
            "lalt")) > 0) || (ct_keyboard_a0.button_state(0,
                                                          ct_keyboard_a0.button_index(
                                                                  "ralt")) > 0);
    io.KeySuper = (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index(
            "super")) > 0);

    char *txt = ct_keyboard_a0.text(0);
    if (txt[0]) {
        io.AddInputCharactersUTF8(txt);
    }

    imguiBeginFrame(mp[0], h - mp[1], btn, wheel[1], w, h, 0, viewid);

    for (uint32_t i = 0; i < ct_array_size(_G.on_debugui); ++i) {
        _G.on_debugui[i]();
    }

    imguiEndFrame();
}

typedef void (*on_debugui)();

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        ct_array_push(_G.name, name, _G.allocator);                           \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = ct_array_size(_G.name);                             \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            ct_array_pop_back(_G.name);                                       \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(on_debugui, on_debugui);

static void on_render() {
    render(255);
}

static void SaveDock(struct ct_vio *output) {
    char* buffer = NULL;
    ImGui::saveToYaml(&buffer, _G.allocator);

    output->write(output, buffer, 1, strlen(buffer));
}

static void LoadDock(const char *path) {
    ImGui::loadFromYaml(path, &ct_ydb_a0, &ct_yng_a0);
}


static ct_debugui_a0 debugui_api = {
        .render = render,

        .register_on_debugui = register_on_debugui_,
        .unregister_on_debugui = unregister_on_debugui_,

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
        .Image2 = imgui_wrap::Image2,
        .ImageButton = imgui_wrap::ImageButton,
        .Checkbox = ImGui::Checkbox,
        .CheckboxFlags = ImGui::CheckboxFlags,
        .RadioButton = ImGui::RadioButton,
        .RadioButton2 = imgui_wrap::RadioButton2,
        .Combo = ImGui::Combo,
        .Combo2 = imgui_wrap::Combo2,
        .Combo3 = imgui_wrap::Combo3,
        .ColorButton = imgui_wrap::ColorButton,
        .ColorEdit3 = ImGui::ColorEdit3,
        .ColorEdit4 = ImGui::ColorEdit4,
        .ColorEditMode = imgui_wrap::ColorEditMode,
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
        .ValueColor = imgui_wrap::ValueColor,
        .ValueColor2 = imgui_wrap::ValueColor2,
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
        .BeginDock = imgui_wrap::BeginDock,
        .EndDock = ImGui::EndDock,
        .IsWindowFocused = ImGui::IsWindowFocused,
        .IsMouseHoveringWindow = ImGui::IsMouseHoveringWindow,
        .SameLine = ImGui::SameLine,
        .HSplitter = imgui_wrap::HSplitter,
        .VSplitter = imgui_wrap::VSplitter,
        .SaveDock = SaveDock,
        .LoadDock = LoadDock,
        .RootDock = imgui_wrap::RootDock,
        .IsMouseClicked = ImGui::IsMouseClicked,
        .IsMouseDoubleClicked = ImGui::IsMouseDoubleClicked,
};

static void _init(ct_api_a0 *api) {
    api->register_api("ct_debugui_a0", &debugui_api);
    imguiCreate(16);

    _G = {
            .allocator = ct_memory_a0.main_allocator()
    };

    ct_renderer_a0.register_on_render(on_render);

    ImGuiIO &io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = ct_keyboard_a0.button_index("tab");
    io.KeyMap[ImGuiKey_LeftArrow] = ct_keyboard_a0.button_index("left");
    io.KeyMap[ImGuiKey_RightArrow] = ct_keyboard_a0.button_index("right");
    io.KeyMap[ImGuiKey_UpArrow] = ct_keyboard_a0.button_index("up");
    io.KeyMap[ImGuiKey_DownArrow] = ct_keyboard_a0.button_index("down");
    io.KeyMap[ImGuiKey_PageUp] = ct_keyboard_a0.button_index("pageup");
    io.KeyMap[ImGuiKey_PageDown] = ct_keyboard_a0.button_index("pagedown");
    io.KeyMap[ImGuiKey_Home] = ct_keyboard_a0.button_index("home");
    io.KeyMap[ImGuiKey_End] = ct_keyboard_a0.button_index("end");
    io.KeyMap[ImGuiKey_Delete] = ct_keyboard_a0.button_index("delete");
    io.KeyMap[ImGuiKey_Backspace] = ct_keyboard_a0.button_index("backspace");
    io.KeyMap[ImGuiKey_Enter] = ct_keyboard_a0.button_index("return");
    io.KeyMap[ImGuiKey_Escape] = ct_keyboard_a0.button_index("escape");

    io.KeyMap[ImGuiKey_A] = ct_keyboard_a0.button_index("a");
    io.KeyMap[ImGuiKey_C] = ct_keyboard_a0.button_index("c");
    io.KeyMap[ImGuiKey_V] = ct_keyboard_a0.button_index("v");
    io.KeyMap[ImGuiKey_X] = ct_keyboard_a0.button_index("x");
    io.KeyMap[ImGuiKey_Y] = ct_keyboard_a0.button_index("y");
    io.KeyMap[ImGuiKey_Z] = ct_keyboard_a0.button_index("z");

}

static void _shutdown() {
    imguiDestroy();

    ct_array_free(_G.on_debugui, _G.allocator);
    ct_renderer_a0.unregister_on_render(on_render);

    _G = {};
}

CETECH_MODULE_DEF(
        debugui,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_mouse_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yng_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)