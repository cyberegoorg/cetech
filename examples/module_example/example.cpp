#include <celib/macros.h>

#include <cetech/core/log/log.h>
#include <cetech/core/config/config.h>
#include <cetech/core/module/module.h>
#include <cetech/core/api/api_system.h>

#include <cetech/engine/input/input.h>
#include <cetech/engine/application/application.h>
#include <cetech/modules/playground/playground.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/engine/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>

CETECH_DECL_API(ct_log_a0)
CETECH_DECL_API(ct_app_a0)
CETECH_DECL_API(ct_keyboard_a0)
CETECH_DECL_API(ct_playground_a0)
CETECH_DECL_API(ct_debugui_a0)
CETECH_DECL_API(ct_hash_a0)
CETECH_DECL_API(ct_renderer_a0)

void update(float dt) {
    if (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index("v"))) {
        ct_log_a0.info("example", "PO");
        ct_log_a0.error("example", "LICE");
    }

    ///ct_log_a0.debug("example", "%f", dt);
}

static struct ct_playground_module module1 = {
        .name = "Module 1",
        .on_gui = []() {
            ct_debugui_a0.Text("dear imgui, %d", 111);

//            static float v[2] = {100.0f, 100.0f};
//            if(ct_debugui_a0.Button("wwwwww", v)) {
//                ct_log_a0.info("dasdsa", "dsadsadsadsad");
//            }

            static float col[4] = {0.0f, 1.0f, 0.0f, 0.0f};
            ct_debugui_a0.ColorButton(col, 1, 2);

            static float vv;
            ct_debugui_a0.DragFloat("FOO:", &vv, 1.0f, 0.0f, 10000.0f, "%.3f",
                                    1.0f);

            static float col2[4] = {0.0f, 1.0f, 0.0f, 0.0f};
            ct_debugui_a0.ColorEdit3("COLOR", col2);

            static float col3[4] = {0.0f, 1.0f, 0.0f, 0.0f};
            ct_debugui_a0.ColorWheel("WHEEE", col3, 0.2f);
        },
};

static struct ct_playground_module module2 = {
        .name = "Module 2",
        .on_gui = []() {
            ct_debugui_a0.Text("dear imgui, %d", 111);
            ct_debugui_a0.Text("By Omar Cornut and all github contributors.");
            ct_debugui_a0.Text(
                    "ImGui is licensed under the MIT License, see LICENSE for more information.");
        },
};

//==============================================================================
// Module def
//==============================================================================
CETECH_MODULE_DEF(
        example,

//==============================================================================
// Init api
//==============================================================================
        {
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_renderer_a0);
        },

//==============================================================================
// Load
//==============================================================================
        {
            CEL_UNUSED(api);

            ct_log_a0.info("example", "Init %d", reload);
            ct_app_a0.register_on_update(update);


            if (reload) {
                ct_playground_a0.unregister_module(module1);
                ct_playground_a0.unregister_module(module2);
            }

            ct_playground_a0.register_module(module1);
            ct_playground_a0.register_module(module2);
        },

//==============================================================================
// Unload
//==============================================================================
        {
            CEL_UNUSED(api);

            ct_log_a0.info("example", "Shutdown %d", reload);
            ct_app_a0.unregister_on_update(update);

            if (!reload) {
                ct_playground_a0.unregister_module(module1);
                ct_playground_a0.unregister_module(module2);
            }
        }
)
