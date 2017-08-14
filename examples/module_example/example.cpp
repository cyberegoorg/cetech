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
#include <cetech/modules/transform/transform.h>
#include <celib/fpumath.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/level/level.h>

CETECH_DECL_API(ct_log_a0)
CETECH_DECL_API(ct_app_a0)
CETECH_DECL_API(ct_keyboard_a0)
CETECH_DECL_API(ct_playground_a0)
CETECH_DECL_API(ct_debugui_a0)
CETECH_DECL_API(ct_hash_a0)

CETECH_DECL_API(ct_renderer_a0)

CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_level_a0);


static struct G {
    ct_viewport viewport;
    ct_world world;
    ct_camera camera;
    ct_entity camera_ent;
} _G;

void fps_camera_update(ct_world world,
                       ct_entity camera_ent,
                       float dt,
                       float dx,
                       float dy,
                       float updown,
                       float leftright,
                       bool fly_mode) {
    float pos[3];
    float rot[4];
    float wm[16];

    auto transform = ct_transform_a0.get(world, camera_ent);

    ct_transform_a0.get_position(transform, pos);
    ct_transform_a0.get_rotation(transform, rot);
    ct_transform_a0.get_world_matrix(transform, wm);

    float x_dir[4];
    float z_dir[4];
    celib::vec4_move(x_dir, &wm[0 * 4]);
    celib::vec4_move(z_dir, &wm[3 * 4]);


    if (fly_mode) {
        z_dir[1] = 0.0f;
    }

    // POS
    float x_dir_new[3];
    float z_dir_new[3];

    celib::vec3_mul(x_dir_new, x_dir, dt * leftright);
    celib::vec3_mul(z_dir_new, z_dir, dt * updown);

    celib::vec3_add(pos, pos, x_dir_new);
    celib::vec3_add(pos, pos, z_dir_new);

    // ROT
//    float rotation_around_world_up[4];
//    float rotation_around_camera_right[4];
//
//
//
//    local rotation_around_world_up = Quatf.from_axis_angle(Vec3f.unit_y(), -dx * dt * 100)
//    local rotation_around_camera_right = Quatf.from_axis_angle(x_dir, dy * dt * 100)
//    local rotation = rotation_around_world_up * rotation_around_camera_right
//
//    Transform.set_position(self.transform, pos)
//    Transform.set_rotation(self.transform, rot * rotation)
//    end

   // ct_transform_a0.set_position(transform, pos);
}


void update(float dt) {
    if (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index("v"))) {
        ct_log_a0.info("example", "PO");
        ct_log_a0.error("example", "LICE");
    }

    ///ct_log_a0.debug("example", "%f", dt);
}

void module1() {
    static bool visible = true;
    if (ct_debugui_a0.BeginDock("Module 1", &visible,
                                DebugUIWindowFlags_Empty)) {


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

    }
    ct_debugui_a0.EndDock();

}

void module2() {
    static bool visible = true;
    if (ct_debugui_a0.BeginDock("Module 2", &visible,
                                DebugUIWindowFlags_Empty)) {
        ct_debugui_a0.Text("dear imgui, %d", 111);
        ct_debugui_a0.Text("By Omar Cornut and all github contributors.");
        ct_debugui_a0.Text(
                "ImGui is licensed under the MIT License, see LICENSE for more information.");
    }
    ct_debugui_a0.EndDock();
}



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


            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_level_a0);
        },

//==============================================================================
// Load
//==============================================================================
        {
            CEL_UNUSED(api);

            ct_log_a0.info("example", "Init %d", reload);

            ct_app_a0.register_on_update(update);

            ct_debugui_a0.register_on_gui(module1);
            ct_debugui_a0.register_on_gui(module2);
        },

//==============================================================================
// Unload
//==============================================================================
        {
            CEL_UNUSED(api);

            ct_log_a0.info("example", "Shutdown %d", reload);

            ct_app_a0.unregister_on_update(update);

            ct_debugui_a0.unregister_on_gui(module1);
            ct_debugui_a0.unregister_on_gui(module2);
        }
)
