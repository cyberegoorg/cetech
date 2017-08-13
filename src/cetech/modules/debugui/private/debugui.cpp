#include <cetech/engine/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/core/blob/blob.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/engine/input/input.h>
#include <cetech/engine/application/application.h>
#include <cetech/core/hashlib/hashlib.h>
#include "celib/map.inl"

#include "cetech/core/config/config.h"
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

using namespace celib;

static struct DebugUIGlobal {
    Array<void (*)()> on_gui;
} _G;

namespace debugui {
    void render(uint8_t viewid) {
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

        imguiBeginFrame(mp[0], h - mp[1], btn, 0, w, h, 0, viewid);


        for (uint32_t i = 0; i < array::size(_G.on_gui); ++i) {
            _G.on_gui[i]();
        }

//        auto th = ct_renderer_a0.get_global_resource(
//                ct_hash_a0.id64_from_str("colorbuffer"));
//        static bool foo = true;
//        ImGui::BeginDock("render2", &foo);
//        imgui_wrap::Image2(th,
//                           (float[2]) {float(w), float(h)},
//                           (float[2]) {0.0f, 0.0f},
//                           (float[2]) {1.0f, 1.0f},
//                           (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
//                           (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
//        ImGui::EndDock();

        imguiEndFrame();
    }

    void register_on_gui(void (*on_gui)()) {
        array::push_back(_G.on_gui, on_gui);
    }
}

static void on_render() {
    debugui::render(255);
}

namespace debugui_module {

    static ct_debugui_a0 debugui_api = {
            .render = debugui::render,
            .register_on_gui = debugui::register_on_gui,

            .Text = imgui_wrap::Text,
            .TextV = imgui_wrap::TextV,
            .TextColored = imgui_wrap::TextColored,
            .TextColoredV = imgui_wrap::TextColoredV,
            .TextDisabled = imgui_wrap::TextDisabled,
            .TextDisabledV = imgui_wrap::TextDisabledV,
            .TextWrapped = imgui_wrap::TextWrapped,
            .TextWrappedV = imgui_wrap::TextWrappedV,
            .TextUnformatted = imgui_wrap::TextUnformatted,
            .LabelText = imgui_wrap::LabelText,
            .LabelTextV = imgui_wrap::LabelTextV,
            .Bullet = imgui_wrap::Bullet,
            .BulletText = imgui_wrap::BulletText,
            .BulletTextV = imgui_wrap::BulletTextV,
            .Button = imgui_wrap::Button,
            .SmallButton = imgui_wrap::SmallButton,
            .InvisibleButton = imgui_wrap::InvisibleButton,
            .Image = imgui_wrap::Image,
            .Image2 = imgui_wrap::Image2,
            .ImageButton = imgui_wrap::ImageButton,
            .Checkbox = imgui_wrap::Checkbox,
            .CheckboxFlags = imgui_wrap::CheckboxFlags,
            .RadioButton = imgui_wrap::RadioButton,
            .RadioButton2 = imgui_wrap::RadioButton2,
            .Combo = imgui_wrap::Combo,
            .Combo2 = imgui_wrap::Combo2,
            .Combo3 = imgui_wrap::Combo3,
            .ColorButton = imgui_wrap::ColorButton,
            .ColorEdit3 = imgui_wrap::ColorEdit3,
            .ColorEdit4 = imgui_wrap::ColorEdit4,
            .ColorEditMode = imgui_wrap::ColorEditMode,
            .PlotLines = imgui_wrap::PlotLines,
            .PlotLines2 = imgui_wrap::PlotLines2,
            .PlotHistogram = imgui_wrap::PlotHistogram,
            .PlotHistogram2 = imgui_wrap::PlotHistogram2,
            .ProgressBar = imgui_wrap::ProgressBar,
            .DragFloat = imgui_wrap::DragFloat,
            .DragFloat2 = imgui_wrap::DragFloat2,
            .DragFloat3 = imgui_wrap::DragFloat3,
            .DragFloat4 = imgui_wrap::DragFloat4,
            .DragFloatRange2 = imgui_wrap::DragFloatRange2,
            .DragInt = imgui_wrap::DragInt,
            .DragInt2 = imgui_wrap::DragInt2,
            .DragInt3 = imgui_wrap::DragInt3,
            .DragInt4 = imgui_wrap::DragInt4,
            .DragIntRange2 = imgui_wrap::DragIntRange2,
            .InputText = imgui_wrap::InputText,
            .InputTextMultiline = imgui_wrap::InputTextMultiline,
            .InputFloat = imgui_wrap::InputFloat,
            .InputFloat2 = imgui_wrap::InputFloat2,
            .InputFloat3 = imgui_wrap::InputFloat3,
            .InputFloat4 = imgui_wrap::InputFloat4,
            .InputInt = imgui_wrap::InputInt,
            .InputInt2 = imgui_wrap::InputInt2,
            .InputInt3 = imgui_wrap::InputInt3,
            .InputInt4 = imgui_wrap::InputInt4,
            .SliderFloat = imgui_wrap::SliderFloat,
            .SliderFloat2 = imgui_wrap::SliderFloat2,
            .SliderFloat3 = imgui_wrap::SliderFloat3,
            .SliderFloat4 = imgui_wrap::SliderFloat4,
            .SliderAngle = imgui_wrap::SliderAngle,
            .SliderInt = imgui_wrap::SliderInt,
            .SliderInt2 = imgui_wrap::SliderInt2,
            .SliderInt3 = imgui_wrap::SliderInt3,
            .SliderInt4 = imgui_wrap::SliderInt4,
            .VSliderFloat = imgui_wrap::VSliderFloat,
            .VSliderInt = imgui_wrap::VSliderInt,
            .TreeNode = imgui_wrap::TreeNode,
            .TreeNode2 = imgui_wrap::TreeNode2,
            .TreeNode3 = imgui_wrap::TreeNode3,
            .TreeNodeV = imgui_wrap::TreeNodeV,
            .TreeNodeV2 = imgui_wrap::TreeNodeV2,
            .TreeNodeEx = imgui_wrap::TreeNodeEx,
            .TreeNodeEx2 = imgui_wrap::TreeNodeEx2,
            .TreeNodeEx3 = imgui_wrap::TreeNodeEx3,
            .TreeNodeExV = imgui_wrap::TreeNodeExV,
            .TreeNodeExV2 = imgui_wrap::TreeNodeExV2,
            .TreePush = imgui_wrap::TreePush,
            .TreePush2 = imgui_wrap::TreePush2,
            .TreePop = imgui_wrap::TreePop,
            .TreeAdvanceToLabelPos = imgui_wrap::TreeAdvanceToLabelPos,
            .GetTreeNodeToLabelSpacing = imgui_wrap::GetTreeNodeToLabelSpacing,
            .SetNextTreeNodeOpen = imgui_wrap::SetNextTreeNodeOpen,
            .CollapsingHeader = imgui_wrap::CollapsingHeader,
            .CollapsingHeader2 = imgui_wrap::CollapsingHeader2,
            .Selectable = imgui_wrap::Selectable,
            .Selectable2 = imgui_wrap::Selectable2,
            .ListBox = imgui_wrap::ListBox,
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
            .SetTooltip = imgui_wrap::SetTooltip,
            .SetTooltipV = imgui_wrap::SetTooltipV,
            .BeginTooltip = imgui_wrap::BeginTooltip,
            .EndTooltip = imgui_wrap::EndTooltip,
            .BeginMainMenuBar = imgui_wrap::BeginMainMenuBar,
            .EndMainMenuBar = imgui_wrap::EndMainMenuBar,
            .BeginMenuBar = imgui_wrap::BeginMenuBar,
            .EndMenuBar = imgui_wrap::EndMenuBar,
            .BeginMenu = imgui_wrap::BeginMenu,
            .EndMenu = imgui_wrap::EndMenu,
            .MenuItem = imgui_wrap::MenuItem,
            .MenuItem2 = imgui_wrap::MenuItem2,
            .OpenPopup = imgui_wrap::OpenPopup,
            .BeginPopup = imgui_wrap::BeginPopup,
            .BeginPopupModal = imgui_wrap::BeginPopupModal,
            .BeginPopupContextItem = imgui_wrap::BeginPopupContextItem,
            .BeginPopupContextWindow = imgui_wrap::BeginPopupContextWindow,
            .BeginPopupContextVoid = imgui_wrap::BeginPopupContextVoid,
            .EndPopup = imgui_wrap::EndPopup,
            .CloseCurrentPopup = imgui_wrap::CloseCurrentPopup,
            .ColorWheel = imgui_wrap::ColorWheel,
            .ColorWheel2 = imgui_wrap::ColorWheel2,
            .GetWindowSize = imgui_wrap::GetWindowSize,
    };

    static void _init(ct_api_a0 *api) {
        api->register_api("ct_debugui_a0", &debugui_api);
        imguiCreate(16);

        _G = {};
        _G.on_gui.init(ct_memory_a0.main_allocator());

        ct_renderer_a0.register_on_render(on_render);
    }

    static void _shutdown() {
        imguiDestroy();

        _G.on_gui.destroy();
        ct_renderer_a0.unregister_on_render(on_render);

        _G = {};
    }
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
        },
        {
            debugui_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            debugui_module::_shutdown();
        }
)