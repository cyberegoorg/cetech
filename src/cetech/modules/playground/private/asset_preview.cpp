#include <cetech/modules/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/renderer/texture.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/application/application.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/transform/transform.h>
#include <celib/fpumath.h>
#include <cetech/modules/input/input.h>
#include <cetech/modules/renderer/viewport.h>
#include <cetech/modules/playground/asset_preview.h>
#include <cetech/modules/playground/asset_browser.h>
#include <cetech/modules/playground/playground.h>
#include "celib/map.inl"

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/config.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_level_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_playground_a0);

using namespace celib;

static struct globals {
    Map<ct_asset_preview_fce> preview_fce;

    uint64_t active_type;
    uint64_t active_name;
    const char *active_path;

    ct_viewport viewport;
    ct_world world;
    ct_entity camera_ent;
    bool visible;
    bool active;
} _G;


static void fps_camera_update(ct_world world,
                              ct_entity camera_ent,
                              float dt,
                              float dx,
                              float dy,
                              float updown,
                              float leftright,
                              float speed,
                              bool fly_mode) {

    CEL_UNUSED(dx);
    CEL_UNUSED(dy);

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
    celib::vec4_move(z_dir, &wm[2 * 4]);


    if (!fly_mode) {
        z_dir[1] = 0.0f;
    }

    // POS
    float x_dir_new[3];
    float z_dir_new[3];

    celib::vec3_mul(x_dir_new, x_dir, dt * leftright * speed);
    celib::vec3_mul(z_dir_new, z_dir, dt * updown * speed);

    celib::vec3_add(pos, pos, x_dir_new);
    celib::vec3_add(pos, pos, z_dir_new);

    ct_transform_a0.set_position(transform, pos);
}

static void on_debugui() {
    if (ct_debugui_a0.BeginDock("Asset preview", &_G.visible,
                                DebugUIWindowFlags_(
                                        DebugUIWindowFlags_NoScrollbar))) {

        _G.active = ct_debugui_a0.IsMouseHoveringWindow();

        auto th = ct_viewport_a0.get_local_resource(
                _G.viewport, ct_hash_a0.id64_from_str("bb_color"));

        float size[2];
        ct_debugui_a0.GetWindowSize(size);
        ct_viewport_a0.resize(_G.viewport, size[0], size[1]);
        ct_debugui_a0.Image2(th,
                             size,
                             (float[2]) {0.0f, 0.0f},
                             (float[2]) {1.0f, 1.0f},
                             (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                             (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
    }
    ct_debugui_a0.EndDock();

}

static void render() {
    if (_G.visible) {
        ct_camera camera = ct_camera_a0.get(_G.world, _G.camera_ent);
        ct_viewport_a0.render_world(_G.world, camera, _G.viewport);
    }
}

static void set_asset(uint64_t type,
                      uint64_t name,
                      uint64_t root,
                      const char *path) {
    CEL_UNUSED(root);

    if (_G.active_name == name) {
        return;
    }

    ct_asset_preview_fce fce = map::get(_G.preview_fce, _G.active_type,
                                        (ct_asset_preview_fce) {NULL});
    if (fce.unload) {
        fce.unload(_G.active_path, _G.active_type, _G.active_name, _G.world);
    }

    _G.active_type = type;
    _G.active_name = name;
    _G.active_path = path;

    fce = map::get(_G.preview_fce, type, (ct_asset_preview_fce) {NULL});
    if (fce.load) {
        fce.load(path, type, name, _G.world);
    }
}

static void init() {
    _G.visible = true;

    _G.viewport = ct_viewport_a0.create(ct_hash_a0.id64_from_str("default"), 0,
                                        0);

    _G.world = ct_world_a0.create();

    _G.camera_ent = ct_entity_a0.spawn(_G.world,
                                       ct_hash_a0.id64_from_str(
                                               "content/camera"));

    ct_transform t = ct_transform_a0.get(_G.world, _G.camera_ent);
    ct_transform_a0.set_position(t, (float[3]) {0.0f, 0.0f, -10.0f});
}

static void shutdown() {

}

static void update(float dt) {
    if (!_G.active) {
        return;
    }

    float updown = 0.0f;
    float leftright = 0.0f;

    auto up_key = ct_keyboard_a0.button_index("w");
    auto down_key = ct_keyboard_a0.button_index("s");
    auto left_key = ct_keyboard_a0.button_index("a");
    auto right_key = ct_keyboard_a0.button_index("d");

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


void register_type_preview(uint64_t type,
                           ct_asset_preview_fce fce) {
    map::set(_G.preview_fce, type, fce);
}

void unregister_type_preview(uint64_t type) {
    map::remove(_G.preview_fce, type);
}

static ct_asset_preview_a0 asset_preview_api = {
        .register_type_preview = register_type_preview,
        .unregister_type_preview = unregister_type_preview
};

static void _init(ct_api_a0 *api) {
    _G = {};

    _G.preview_fce.init(ct_memory_a0.main_allocator());

    ct_playground_a0.register_module(
            ct_hash_a0.id64_from_str("asset_preview"),
            (ct_playground_module_fce){
                    .on_init = init,
                    .on_shutdown = shutdown,
                    .on_render = render,
                    .on_update = update,
                    .on_ui = on_debugui,
            });

    ct_asset_browser_a0.register_on_asset_click(set_asset);

    api->register_api("ct_asset_preview_a0", &asset_preview_api);
}

static void _shutdown() {

    ct_playground_a0.unregister_module(
            ct_hash_a0.id64_from_str("asset_preview")
    );

    ct_asset_browser_a0.unregister_on_asset_click(set_asset);

    _G.preview_fce.destroy();

    _G = {};
}

CETECH_MODULE_DEF(
        asset_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_level_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_playground_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)