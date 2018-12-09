#define CE_DYNAMIC_MODULE

#include <celib/macros.h>

#include <celib/log.h>
#include <celib/config.h>
#include <celib/module.h>
#include <celib/api_system.h>
#include <celib/hashlib.h>

#include <cetech/controlers/keyboard.h>
#include <celib/ebus.h>
#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>

#include <cetech/debugui/debugui.h>
#include <cetech/renderer/renderer.h>
#include <cetech/transform/transform.h>
#include <cetech/texture/texture.h>
#include <cetech/kernel/kernel.h>

#include <stdlib.h>
#include <cetech/controlers/controlers.h>
#include <string.h>

static struct G {
    struct ct_world world;
    struct ct_entity camera_ent;
    float dt;
} _G;

void update(uint64_t type, void* event) {
    struct ebus_cdb_event *ev = event;

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(ev->obj);
    _G.dt = ce_cdb_a0->read_float(reader, ce_id_a0->id64("dt"), 0.0f);

    struct ct_controlers_i0* keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);


    if (keyboard->button_state(0, keyboard->button_index("v"))) {
        ce_log_a0->info("example", "PO");
        ce_log_a0->error("example", "LICE");
    }
    ///ct_log_a0->debug("example", "%f", dt);
}

void module1(uint64_t type, void* event) {
    CE_UNUSED(event);

    static bool visible = true;
    if (ct_debugui_a0->BeginDock("Module 1", &visible, 0)) {

        ct_debugui_a0->Text("DT: %f", _G.dt);
        ct_debugui_a0->Text("FPS: %f", 1.0f / _G.dt );
        ct_debugui_a0->Text("Raddnddddom FPS: %f", (double) rand());

        float fps =  (1.0f / _G.dt);

        ct_debugui_a0->PlotLines("FPS", &fps, 1, 0, NULL, 0.0f, 0.0f, (float[2]){100.0f, 50.0f}, 0);

        ct_debugui_a0->Text("xknalsnxlsanlknxlasnlknxslknsaxdear imgui, %d", 111);

            static float v[2] = {100.0f, 100.0f};
            if(ct_debugui_a0->Button("sjdoiasjwww", v)) {
                ce_log_a0->debug("dasdsa", "dsadsdsadsadsadasd");
            }

//        static float col[4] = {0.0f, 1.0f, 0.0f, 0.0f};
//        ct_debugui_a0->ColorButton(col, 1, 2);

        static float vv;
        ct_debugui_a0->DragFloat("FOO:", &vv, 1.0f, 0.0f, 10000.0f, "%.3f",
                                1.0f);

        static float col2[4] = {0.0f, 1.0f, 0.0f, 0.0f};
        ct_debugui_a0->ColorEdit3("COLOR", col2, 0);

        static float col3[4] = {0.0f, 1.0f, 0.0f, 0.0f};
        ct_debugui_a0->ColorWheel("WHEEE", col3, 0.2f);


        float size[2] = {};
        ct_debugui_a0->GetWindowSize(size);
        size[1] = size[0];
        ct_debugui_a0->Image2(ct_texture_a0->get(ce_id_a0->id64("content/scene/duck/duckCM")),//"content/scene/m4a1/m4_diff"
                             size,
                             (float[2]) {0.0f, 0.0f},
                             (float[2]) {1.0f, 1.0f},
                             (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                             (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
    }
    ct_debugui_a0->EndDock();

}

void module2() {
    static bool visible = true;
    if (ct_debugui_a0->BeginDock("Module 2", &visible, 0)) {
        ct_debugui_a0->Text("dear imgui, %d", 111);
        ct_debugui_a0->Text("By Omar Cornut and all github contributors.");
        ct_debugui_a0->Text(
                "ImGui is licensed under the MIT License, see LICENSE for more information.");
    }
    ct_debugui_a0->EndDock();
}



//==============================================================================
// Module def
//==============================================================================
CE_MODULE_DEF(
        example,

//==============================================================================
// Init api
//==============================================================================
        {
            CE_INIT_API(api, ct_controlers_a0);
            CE_INIT_API(api, ce_log_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_renderer_a0);
            CE_INIT_API(api, ce_ebus_a0);

            CE_INIT_API(api, ct_ecs_a0);
//            CE_INIT_API(api, ct_camera_a0);
            CE_INIT_API(api, ct_texture_a0);
            CE_INIT_API(api, ce_cdb_a0);

        },

//==============================================================================
// Load
//==============================================================================
        {
            CE_UNUSED(api);

            ce_log_a0->info("example", "Init %d", reload);

            ce_ebus_a0->connect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, update, 0);
            ce_ebus_a0->connect(DEBUGUI_EBUS, DEBUGUI_EVENT, module1, 0);

//            ct_debugui_a0->register_on_debugui(module1);
            //ct_debugui_a0->register_on_debugui(module2);
        },

//==============================================================================
// Unload
//==============================================================================
        {
            CE_UNUSED(api);

            ce_log_a0->info("example", "Shutdown %d", reload);

            ce_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, update);
            ce_ebus_a0->disconnect(DEBUGUI_EBUS, DEBUGUI_EVENT, module1);
//            ct_debugui_a0->unregister_on_debugui(module1);
//            ct_debugui_a0->unregister_on_debugui(module2);
        }
)


