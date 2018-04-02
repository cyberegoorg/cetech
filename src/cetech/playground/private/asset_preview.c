#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/viewport.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/controlers/keyboard.h>
#include <cetech/engine/debugdraw/debugdraw.h>

#include <cetech/playground/asset_preview.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/playground.h>
#include <cetech/core/containers/hash.h>
#include <cetech/core/math/fmath.h>
#include <cetech/core/ebus/ebus.h>

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_dd_a0);

#define _G AssetPreviewGlobals

static struct _G {
    struct ct_alloc *allocator;
    struct ct_hash_t preview_fce_map;
    struct ct_asset_preview_fce *preview_fce;

    struct ct_resource_id active_asset;

    const char *active_path;

    struct ct_world world;
    struct ct_entity camera_ent;
    bool visible;
    bool active;
} _G;


static void fps_camera_update(struct ct_world world,
                              struct ct_entity camera_ent,
                              float dt,
                              float dx,
                              float dy,
                              float updown,
                              float leftright,
                              float speed,
                              bool fly_mode) {

    CT_UNUSED(dx);
    CT_UNUSED(dy);

    float wm[16];


    struct ct_transform_comp *transform;
    transform = ct_ecs_a0.entity_data(world, TRANSFORM_COMPONENT, camera_ent);

    ct_mat4_move(wm, transform->world);

    float x_dir[4];
    float z_dir[4];
    ct_vec4_move(x_dir, &wm[0 * 4]);
    ct_vec4_move(z_dir, &wm[2 * 4]);

    if (!fly_mode) {
        z_dir[1] = 0.0f;
    }

    // POS
    float x_dir_new[3];
    float z_dir_new[3];

    ct_vec3_mul_s(x_dir_new, x_dir, dt * leftright * speed);
    ct_vec3_mul_s(z_dir_new, z_dir, dt * updown * speed);

    ct_vec3_add(transform->position, transform->position, x_dir_new);
    ct_vec3_add(transform->position, transform->position, z_dir_new);

    ct_ecs_a0.entity_component_change(world, TRANSFORM_COMPONENT, camera_ent);

    // ROT
//    float rotation_around_world_up[4];
//    float rotation_around_camera_right[4];
//
//    local rotation_around_world_up = Quatf.from_axis_angle(Vec3f.unit_y(), -dx * dt * 100)
//    local rotation_around_camera_right = Quatf.from_axis_angle(x_dir, dy * dt * 100)
//    local rotation = rotation_around_world_up * rotation_around_camera_right
//
//    Transform.set_position(self.transform, pos)
//    Transform.set_rotation(self.transform, rot * rotation)
//    end
}

static void on_debugui(uint32_t bus_name,
                       void *event) {
    if (ct_debugui_a0.BeginDock("Asset preview", &_G.visible,
                                DebugUIWindowFlags_NoScrollbar)) {

        _G.active = ct_debugui_a0.IsMouseHoveringWindow();

        struct ct_camera_component *camera_data;
        camera_data = ct_ecs_a0.entity_data(_G.world, CAMERA_COMPONENT,
                                            _G.camera_ent);

        struct ct_texture th = ct_viewport_a0.get_local_resource(
                camera_data->viewport,
                CT_ID64_0("bb_color"));

        float size[2];
        ct_debugui_a0.GetWindowSize(size);
        ct_viewport_a0.resize(camera_data->viewport, size[0], size[1]);
        ct_debugui_a0.Image2(th,
                             size,
                             (float[2]) {0.0f, 0.0f},
                             (float[2]) {1.0f, 1.0f},
                             (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                             (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
    }
    ct_debugui_a0.EndDock();

}


static void set_asset(uint32_t bus_name,
                      void *event) {

    struct ct_asset_browser_click_ev *ev = event;
    struct ct_resource_id rid = {.i64 = ev->asset};

    if (_G.active_asset.i64 == rid.i64) {
        return;
    }

    uint64_t idx = ct_hash_lookup(&_G.preview_fce_map, _G.active_asset.type,
                                  UINT64_MAX);
    if (idx != UINT64_MAX) {
        struct ct_asset_preview_fce fce = _G.preview_fce[idx];

        if (fce.unload) {
            fce.unload(_G.active_path, _G.active_asset, _G.world);
        }
    }

    _G.active_asset = rid;
    _G.active_path = ev->path;

    idx = ct_hash_lookup(&_G.preview_fce_map, rid.type, UINT64_MAX);
    if (idx != UINT64_MAX) {
        struct ct_asset_preview_fce fce = _G.preview_fce[idx];

        if (fce.load) {
            fce.load(ev->path, rid, _G.world);
        }
    }

    struct ct_transform_comp *transform;
    transform = ct_ecs_a0.entity_data(_G.world,
                                      TRANSFORM_COMPONENT,
                                      _G.camera_ent);

//    ct_vec3_move(transform->position, (float[3]) {0.0f, 0.0f, -10.0f});
}

static void init(uint32_t bus_name,
                 void *event) {
    _G.visible = true;
    _G.world = ct_ecs_a0.create_world();
    _G.camera_ent = ct_ecs_a0.spawn_entity(_G.world,
                                           CT_ID32_0("content/camera"));

    ct_ecs_a0.add_simulation(_G.world, CT_ID64_0("render"));
}


static void update(uint32_t bus_name,
                   void *event) {
    struct ct_playground_update_ev *ev = event;
    float dt = ev->dt;

    if (_G.active) {
        float updown = 0.0f;
        float leftright = 0.0f;

        uint32_t up_key = ct_keyboard_a0.button_index("w");
        uint32_t down_key = ct_keyboard_a0.button_index("s");
        uint32_t left_key = ct_keyboard_a0.button_index("a");
        uint32_t right_key = ct_keyboard_a0.button_index("d");

        if (ct_keyboard_a0.button_state(0, up_key) > 0) {
            updown = 1.0f;
        }

        if (ct_keyboard_a0.button_state(0, down_key) > 0) {
            updown = -1.0f;
        }

        if (ct_keyboard_a0.button_state(0, right_key) > 0) {
            leftright = 1.0f;
        }

        if (ct_keyboard_a0.button_state(0, left_key) > 0) {
            leftright = -1.0f;
        }

        fps_camera_update(_G.world, _G.camera_ent, dt,
                          0, 0, updown, leftright, 10.0f, false);
    }

    if (_G.visible) {
        ct_ecs_a0.simulate(_G.world, dt);
    }
}

#define ct_instance_map(a, h, k, item, al) \
    ct_array_push(a, item, al); \
    ct_hash_add(h, k, ct_array_size(a) - 1, al)

void register_type_preview(const char *type,
                           struct ct_asset_preview_fce fce) {
    uint32_t id = CT_ID32_0(type);
    ct_instance_map(_G.preview_fce, &_G.preview_fce_map, id, fce,
                    _G.allocator);
}

void unregister_type_preview(const char *type) {
    uint32_t id = CT_ID32_0(type);

    uint64_t idx = ct_hash_lookup(&_G.preview_fce_map, id, UINT64_MAX);
    if (UINT64_MAX == idx) {
        return;
    }
    ct_hash_remove(&_G.preview_fce_map, id);
}

static void on_menu_window(uint32_t bus_name,
                           void *event) {
    ct_debugui_a0.MenuItem2("Asset preview", NULL, &_G.visible, true);
}

static struct ct_asset_preview_a0 asset_preview_api = {
        .register_type_preview = register_type_preview,
        .unregister_type_preview = unregister_type_preview
};

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator()
    };

    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_INIT_EVENT, init, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UPDATE_EVENT, update, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT,
                       on_menu_window, 0);

    ct_ebus_a0.connect(ASSET_BROWSER_EBUS, ASSET_CLICK_EVENT, set_asset, 0);


    api->register_api("ct_asset_preview_a0", &asset_preview_api);
}

static void _shutdown() {
    ct_hash_free(&_G.preview_fce_map, _G.allocator);
    ct_array_free(_G.preview_fce, _G.allocator);

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        asset_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_dd_a0);
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